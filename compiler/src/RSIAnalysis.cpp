#include "R-Sharp/RSIAnalysis.hpp"
#include "R-Sharp/RSIGenerator.hpp"
#include "R-Sharp/Graph.hpp"
#include "R-Sharp/Architecture.hpp"
#include "R-Sharp/Utils/ContainerTools.hpp"

#include "R-Sharp/Utils/ScopeGuard.hpp"


namespace RSI {

void analyzeLiveVariables(RSI::Function& function, Architecture const&) {
    bool hasModifiedTheIR = true;

    const auto setSizeWatchGuard = [&hasModifiedTheIR](auto const& setToWatch) {
        const auto sizeAtBeginning = setToWatch.size();
        return ScopeGuard([&setToWatch, sizeAtBeginning, &hasModifiedTheIR]() {
            if (sizeAtBeginning != setToWatch.size()) {
                hasModifiedTheIR = true;
            }
        });
    };

    Instruction virtualEndInstruction{
        .type = InstructionType::NOP,
    };
    while (hasModifiedTheIR) {
        hasModifiedTheIR = false;

        Instruction* lastInstruction = &virtualEndInstruction;
        for (auto it = function.instructions.rbegin(); it != function.instructions.rend(); it++) {
            auto& instr = *it;
            const auto liveVariableBeforeThisPass = instr.meta.liveVariablesBefore;

            if (lastInstruction->meta.liveVariablesBefore.size() != 0) {
                instr.meta.liveVariablesBefore.insert(
                    lastInstruction->meta.liveVariablesBefore.begin(),
                    lastInstruction->meta.liveVariablesBefore.end()
                );
            }

            if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.result)) {
                instr.meta.liveVariablesBefore.erase(std::get<std::shared_ptr<RSI::Reference>>(instr.result));
            }

            if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.op1)) {
                instr.meta.liveVariablesBefore.insert(std::get<std::shared_ptr<RSI::Reference>>(instr.op1));
            }
            if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.op2)) {
                instr.meta.liveVariablesBefore.insert(std::get<std::shared_ptr<RSI::Reference>>(instr.op2));
            }

            if (instr.type == RSI::InstructionType::DEFINE_LABEL) {
                for (auto& jump_instr : function.instructions) {
                    if (jump_instr.type != RSI::InstructionType::JUMP && jump_instr.type != RSI::InstructionType::JUMP_IF_ZERO)
                        continue;

                    if (jump_instr.op1 == instr.op1 || jump_instr.op2 == instr.op1) {
                        const auto guard = setSizeWatchGuard(jump_instr.meta.liveVariablesBefore);
                        jump_instr.meta.liveVariablesBefore.insert(
                            instr.meta.liveVariablesBefore.begin(), instr.meta.liveVariablesBefore.end()
                        );
                    }
                }
            }

            if (liveVariableBeforeThisPass != instr.meta.liveVariablesBefore) {
                hasModifiedTheIR = true;
            }

            lastInstruction = &*it;
        }
        Print("LVA Pass");
    }
}

void assignRegistersGraphColoring(Function& func, Architecture const& arch) {
    Graph<void> interferenceGraph;
    using Vertex = Vertex<void>;

    auto allAssignableColors = VertexColor::getNColors(arch.generalPurposeRegisters.size());
    auto stackPointerColor = VertexColor();
    auto allAvailableColors = allAssignableColors;
    allAvailableColors.push_back(stackPointerColor);

    std::map<VertexColor, RSI::HWRegister> colorToHWRegister;
    std::map<RSI::HWRegister, VertexColor> HWRegisterToColor;

    for (int i = 0; i < arch.generalPurposeRegisters.size(); i++) {
        colorToHWRegister.insert({allAssignableColors.at(i), arch.generalPurposeRegisters.at(i)});
        HWRegisterToColor.insert({arch.generalPurposeRegisters.at(i), allAssignableColors.at(i)});
    }
    colorToHWRegister.insert({stackPointerColor, arch.stackPointerRegister});
    HWRegisterToColor.insert({arch.stackPointerRegister, stackPointerColor});

    std::map<std::shared_ptr<RSI::Reference>, std::shared_ptr<Vertex>> referenceToVertex;
    std::map<std::shared_ptr<Vertex>, std::shared_ptr<RSI::Reference>> vertexToReference;

    const auto addVertexToGraph = [&](auto& operand) {
        if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(operand)) {
            std::shared_ptr<Vertex> vert = std::make_shared<Vertex>();
            auto ref = std::get<std::shared_ptr<RSI::Reference>>(operand);

            // keep the color/register
            if (std::holds_alternative<RSI::HWRegister>(ref->storageLocation)) {
                vert->color = HWRegisterToColor.at(std::get<RSI::HWRegister>(ref->storageLocation));
            }
            if (std::holds_alternative<RSI::StackSlot>(ref->storageLocation)) {
                vert->color = VertexColor();
            }
            if (referenceToVertex.count(ref) == 0) {
                referenceToVertex.insert_or_assign(ref, vert);
                vertexToReference.insert_or_assign(vert, ref);
                interferenceGraph.addVertex(vert);
            }
        }
    };

    std::optional<std::reference_wrapper<RSI::Instruction>> lastInstruction;
    for (auto& instr : func.instructions) {
        addVertexToGraph(instr.result);
        addVertexToGraph(instr.op1);
        addVertexToGraph(instr.op2);

        if (lastInstruction.has_value()
            && std::holds_alternative<std::shared_ptr<RSI::Reference>>(lastInstruction.value().get().result)) {
            for (auto liveVar : instr.meta.liveVariablesBefore) {
                if (liveVar == std::get<std::shared_ptr<RSI::Reference>>(lastInstruction.value().get().result))
                    continue;

                interferenceGraph.addEdge(
                    referenceToVertex.at(liveVar),
                    referenceToVertex.at(
                        std::get<std::shared_ptr<RSI::Reference>>(lastInstruction.value().get().result)
                    )
                );
            }
        }

        for (auto liveVar1 : instr.meta.liveVariablesBefore) {
            for (auto liveVar2 : instr.meta.liveVariablesBefore) {
                if (liveVar1 == liveVar2) continue;

                interferenceGraph.addEdge(referenceToVertex.at(liveVar1), referenceToVertex.at(liveVar2));
            }
        }

        lastInstruction = instr;
    }

    interferenceGraph.colorIn(allAssignableColors);

    std::map<VertexColor, RSI::StackSlot> colorToStackSlot;
    uint64_t currentStackOffset = 0;

    for (auto vertex : interferenceGraph) {
        if (!ContainerTools::contains(allAvailableColors, vertex->color.value())) {
            if (colorToStackSlot.count(vertex->color.value()))
                vertexToReference.at(vertex)->storageLocation = colorToStackSlot.at(vertex->color.value());
            else {
                vertexToReference.at(vertex)->storageLocation = StackSlot{.offset = currentStackOffset};
                currentStackOffset += 8;
                colorToStackSlot.insert_or_assign(
                    vertex->color.value(), std::get<RSI::StackSlot>(vertexToReference.at(vertex)->storageLocation)
                );
            }
        }
        else {
            vertexToReference.at(vertex)->storageLocation = colorToHWRegister.at(vertex->color.value());
        }
    }
}

void enumerateRegisters(Function& func, Architecture const& architecture) {
    func.meta.allRegisters = {};
    func.meta.allReferences = {};
    func.meta.maxStackUsage = 0;
    for (auto instr : func.instructions) {
        if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.result))
            func.meta.allReferences.insert(std::get<std::shared_ptr<RSI::Reference>>(instr.result));

        if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.op1))
            func.meta.allReferences.insert(std::get<std::shared_ptr<RSI::Reference>>(instr.op1));

        if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.op2))
            func.meta.allReferences.insert(std::get<std::shared_ptr<RSI::Reference>>(instr.op2));
    }

    for (auto ref : func.meta.allReferences) {
        if (std::holds_alternative<RSI::HWRegister>(ref->storageLocation))
            func.meta.allRegisters.insert(std::get<RSI::HWRegister>(ref->storageLocation));
        if (std::holds_alternative<RSI::StackSlot>(ref->storageLocation)) func.meta.maxStackUsage += 8;
    }
}


bool makeTwoOperandCompatible_prefilter(RSI::Instruction const& instr) {
    if (instr.type == InstructionType::NEGATE) return true;

    if (!std::holds_alternative<std::shared_ptr<Reference>>(instr.result)
        || !std::holds_alternative<std::shared_ptr<Reference>>(instr.op1)
        || std::holds_alternative<std::monostate>(instr.op2))
        return false;

    return instr.result != instr.op1;
}

void makeTwoOperandCompatible(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
) {
    Instruction move{
        .type = InstructionType::MOVE,
        .result = instr.result,
        .op1 = instr.op1,
    };
    instr.op1 = instr.result;
    beforeInstructions.push_back(move);
}

void replaceModWithDivMulSub(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
) {
    instr.type = InstructionType::DIVIDE;

    auto finalResult = instr.result;
    instr.result = std::make_shared<Reference>(Reference{.name = RSIGenerator::makeStringUnique("tmp")});

    Instruction mult{
        .type = InstructionType::MULTIPLY,
        .result = std::make_shared<Reference>(Reference{.name = RSIGenerator::makeStringUnique("tmp")}),
        .op1 = instr.result,
        .op2 = instr.op2,
    };
    Instruction sub{
        .type = InstructionType::SUBTRACT,
        .result = finalResult,
        .op1 = instr.op1,
        .op2 = mult.result,
    };


    afterInstructions.push_back(mult);
    afterInstructions.push_back(sub);
}
void moveConstantsToReferences(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
) {
    if (std::holds_alternative<RSI::Constant>(instr.op1) || std::holds_alternative<RSI::DynamicConstant>(instr.op1)) {
        RSI::Instruction move{
            .type = RSI::InstructionType::MOVE,
            .result = RSIGenerator::getNewReference("constant"),
            .op1 = instr.op1,
        };
        instr.op1 = move.result;

        beforeInstructions.push_back(move);
    }

    if (std::holds_alternative<RSI::Constant>(instr.op2) || std::holds_alternative<RSI::DynamicConstant>(instr.op2)) {
        RSI::Instruction move{
            .type = RSI::InstructionType::MOVE,
            .result = RSIGenerator::getNewReference("constant"),
            .op1 = instr.op2,
        };
        instr.op2 = move.result;

        beforeInstructions.push_back(move);
    }
}

void seperateDivReferences(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
) {
    RSI::Instruction move1{
        .type = RSI::InstructionType::MOVE,
        .result = RSIGenerator::getNewReference("divtmp"),
        .op1 = instr.op1,
    };
    RSI::Instruction move2{
        .type = RSI::InstructionType::MOVE,
        .result = RSIGenerator::getNewReference("divtmp"),
        .op1 = instr.op2,
    };
    RSI::Instruction moveRes{
        .type = RSI::InstructionType::MOVE,
        .result = instr.result,
        .op1 = RSIGenerator::getNewReference("divresult"),
    };
    instr.op1 = move1.result;
    instr.op2 = move2.result;
    instr.result = moveRes.op1;

    std::get<std::shared_ptr<RSI::Reference>>(instr.op1)->storageLocation = x86_64.allRegisters.at(
        static_cast<int>(NasmRegisters::RAX)
    );
    std::get<std::shared_ptr<RSI::Reference>>(instr.result)->storageLocation = x86_64.allRegisters.at(
        static_cast<int>(NasmRegisters::RAX)
    );

    beforeInstructions.push_back(move1);
    beforeInstructions.push_back(move2);
    afterInstructions.push_back(moveRes);
}

void seperateCallResults(
    Architecture const& architecture,
    RSI::Instruction& instr,
    std::vector<RSI::Instruction>& beforeInstructions,
    std::vector<RSI::Instruction>& afterInstructions
) {
    RSI::Instruction moveRes{
        .type = RSI::InstructionType::MOVE,
        .result = instr.result,
        .op1 = RSIGenerator::getNewReference("callresult"),
    };
    instr.result = moveRes.op1;

    std::get<std::shared_ptr<RSI::Reference>>(instr.result)->storageLocation = architecture.returnValueRegister;

    afterInstructions.push_back(moveRes);
}

void separateLoadParameters(
    Architecture const& architecture,
    RSI::Instruction& instr,
    std::vector<RSI::Instruction>& beforeInstructions,
    std::vector<RSI::Instruction>& afterInstructions
) {
    uint parameterIndex = std::get<RSI::Constant>(instr.op1).value;
    if (parameterIndex >= architecture.parameterRegisters.size())
        Fatal("Function uses more parameters than are supported on this platform");

    const auto param = RSIGenerator::getNewReference("param");
    param->storageLocation = architecture.parameterRegisters.at(parameterIndex);

    RSI::Instruction move{
        .type = RSI::InstructionType::MOVE,
        .result = instr.result,
        .op1 = param,
    };

    instr.result = param;
    afterInstructions.push_back(move);
}

void resolveAddressOf(
    Architecture const& architecture,
    RSI::Instruction& instr,
    std::vector<RSI::Instruction>& beforeInstructions,
    std::vector<RSI::Instruction>& afterInstructions
) {
    if (!std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.op1)
        || !std::holds_alternative<RSI::StackSlot>(std::get<std::shared_ptr<RSI::Reference>>(instr.op1)->storageLocation)) {
        Fatal("Trying to take address of value not on stack.");
    }
    auto& op = std::get<RSI::StackSlot>(std::get<std::shared_ptr<RSI::Reference>>(instr.op1)->storageLocation);
    instr.type = RSI::InstructionType::ADD;
    instr.op1 = RSI::DynamicConstant{
        .value = &op.offset,
    };
    auto stack_pointer = RSIGenerator::getNewReference("sp");
    stack_pointer->storageLocation = architecture.stackPointerRegister;
    instr.op2 = stack_pointer;

    beforeInstructions.push_back(Instruction{
        .type = InstructionType::SET_LIVE,
        .result = stack_pointer,
    });
}

void separateGlobalReferences(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
) {
    if (std::holds_alternative<std::shared_ptr<RSI::GlobalReference>>(instr.result)) {
        RSI::Instruction move{
            .type = RSI::InstructionType::MOVE,
            .result = instr.result,
            .op1 = RSIGenerator::getNewReference(),
        };
        instr.result = move.op1;
        afterInstructions.push_back(move);
    }
    if (std::holds_alternative<std::shared_ptr<RSI::GlobalReference>>(instr.op1)) {
        RSI::Instruction move{
            .type = RSI::InstructionType::MOVE,
            .result = RSIGenerator::getNewReference(),
            .op1 = instr.op1,
        };
        instr.op1 = move.result;
        beforeInstructions.push_back(move);
    }
    if (std::holds_alternative<std::shared_ptr<RSI::GlobalReference>>(instr.op2)) {
        RSI::Instruction move{
            .type = RSI::InstructionType::MOVE,
            .result = RSIGenerator::getNewReference(),
            .op1 = instr.op2,
        };
        instr.op2 = move.result;
        beforeInstructions.push_back(move);
    }
}
void globalReferenceToMemoryAccess(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
) {
    if (std::holds_alternative<std::shared_ptr<RSI::GlobalReference>>(instr.result)) {
        RSI::Instruction smem{
            .type = RSI::InstructionType::STORE_MEMORY,
            .op1 = RSIGenerator::getNewReference(),
            .op2 = instr.op1,
        };
        instr.op1 = instr.result;
        instr.result = smem.op1;
        afterInstructions.push_back(smem);
    }
    else if (std::holds_alternative<std::shared_ptr<RSI::GlobalReference>>(instr.op1)) {
        RSI::Instruction lmem{
            .type = RSI::InstructionType::LOAD_MEMORY,
            .result = instr.result,
            .op1 = RSIGenerator::getNewReference(),
        };
        instr.result = lmem.op1;
        afterInstructions.push_back(lmem);
    }
}

void separateStackVariables(
    RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions
) {
    if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.result)
        && std::holds_alternative<RSI::StackSlot>(std::get<std::shared_ptr<RSI::Reference>>(instr.result)->storageLocation)) {
        RSI::Instruction smem{
            .type = RSI::InstructionType::STORE_MEMORY,
            .op1 = RSIGenerator::getNewReference(),
            .op2 = instr.op1,
        };
        instr.op1 = instr.result;
        instr.result = smem.op1;
        afterInstructions.push_back(smem);
    }
}


}