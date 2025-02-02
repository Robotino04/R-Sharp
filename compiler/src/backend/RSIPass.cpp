#include "R-Sharp/backend/RSIPass.hpp"
#include "R-Sharp/backend/RSI.hpp"
#include "R-Sharp/backend/RSITools.hpp"
#include "R-Sharp/Logging.hpp"

void RSIPass::operator()(RSI::Function& function, OutputArchitecture arch) const {
    auto& fullArch = arch == OutputArchitecture::AArch64 ? aarch64 : x86_64;

    if (architectures.count(arch) == 0) return;
    if (isFunctionWide) {
        perFunctionFunction(function, fullArch);
        return;
    }

    int i = 0;
    while (i < function.instructions.size()) {
        if (positiveInstructionTypes.size() && positiveInstructionTypes.count(function.instructions.at(i).type) == 0) {
            i++;
            continue;
        }
        if (negativeInstructionTypes.count(function.instructions.at(i).type) != 0) {
            i++;
            continue;
        }
        if (!prefilter(function.instructions.at(i))) {
            i++;
            continue;
        }

        std::vector<RSI::Instruction> before, after;

        perInstructionFunction(function.instructions.at(i), before, after);
        function.instructions.insert(function.instructions.begin() + i + 1, after.begin(), after.end());
        function.instructions.insert(function.instructions.begin() + i, before.begin(), before.end());

        i += before.size() + after.size();
        i++;
    }
}
void RSIPass::operator()(std::vector<RSI::Function>& functions, OutputArchitecture arch) const {
    auto const& registerTranslation = arch == OutputArchitecture::x86_64 ? x86_64.registerTranslation
                                                                         : aarch64.registerTranslation;

    if (architectures.count(arch) == 0) return;

    bool isSilent = humanHeader.length() == 0;

    if (!isSilent) Print("--------------| ", humanHeader, " |--------------");
    for (auto& func : functions) {
        if (!isSilent) Print("; Function \"", func.name, "\"");
        (*this)(func, arch);
        if (!isSilent) Print(RSI::stringify_function(func, registerTranslation));
    }
}
