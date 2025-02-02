#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "R-Sharp/Logging.hpp"

#include "R-Sharp/frontend/Tokenizer.hpp"
#include "R-Sharp/frontend/Token.hpp"
#include "R-Sharp/frontend/Parser.hpp"
#include "R-Sharp/frontend/Utils.hpp"
#include "R-Sharp/frontend/ParsingCache.hpp"

#include "R-Sharp/ast/AstNodes.hpp"
#include "R-Sharp/ast/AstPrinter.hpp"
#include "R-Sharp/backend/CCodeGenerator.hpp"
#include "R-Sharp/backend/NASMCodeGenerator.hpp"
#include "R-Sharp/backend/AArch64CodeGenerator.hpp"
#include "R-Sharp/ast/ErrorPrinter.hpp"
#include "R-Sharp/ast/SemanticValidator.hpp"
#include "R-Sharp/backend/RSIGenerator.hpp"
#include "R-Sharp/backend/RSIAnalysis.hpp"
#include "R-Sharp/backend/RSIToAssembly.hpp"
#include "R-Sharp/backend/Architecture.hpp"
#include "R-Sharp/backend/RSIPass.hpp"

enum class ReturnValue {
    NormalExit = 0,
    UnknownError = 1,
    SyntaxError = 2,
    SemanticError = 3,
    AssemblingError = 4,
};

void printHelp(const char* programName) {
    std::cout << "Usage: " << programName << " [options] [input file]\n";
    std::cout <<
        R"(Options:
  -h, --help                Print this help message
  -o, --output <file>       Output file
  -f, --format <format>     Output format (c, nasm, aarch64, rsi)
  --compiler <path>         Use this compiler. Default: "gcc"
  --link <file>             Additionally link <file> into the output. Can be repeated.
  --stdlib <path>           Use the standard library at <path>.

Return values:
  0     Everything OK
  1     Something unknown went wrong
  2     There were syntax errors
  3     There were semantic errors
  4     There were assembling errors
)";
}

enum class OutputFormat {
    C,
    NASM,
    AArch64,
    RSI_NASM,
    RSI_AArch64,
};

int main(int argc, const char** argv) {
    std::string inputFilename;
    std::string outputFilename = "a.out";
    OutputFormat outputFormat = OutputFormat::C;
    OutputArchitecture outputArchitecture = OutputArchitecture::x86_64;
    std::string compiler = "gcc";
    std::vector<std::string> additionalyLinkedFiles;
    std::string stdlibIncludePath = std::filesystem::path(argv[0]).replace_filename("stdlib/");

    if (argc < 2) {
        printHelp(argv[0]);
        return static_cast<int>(ReturnValue::UnknownError);
    }

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return static_cast<int>(ReturnValue::NormalExit);
        }
        else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputFilename = argv[i + 1];
                i++;
            }
            else {
                Error("Missing output file");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else if (arg == "--link") {
            if (i + 1 < argc) {
                additionalyLinkedFiles.push_back(argv[i + 1]);
                i++;
            }
            else {
                Error("Missing file to link");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else if (arg == "-f" || arg == "--format") {
            if (i + 1 < argc) {
                std::string format = argv[i + 1];
                if (format == "c") {
                    outputFormat = OutputFormat::C;
                }
                else if (format == "nasm") {
                    outputFormat = OutputFormat::NASM;
                    outputArchitecture = OutputArchitecture::x86_64;
                }
                else if (format == "aarch64") {
                    outputFormat = OutputFormat::AArch64;
                    outputArchitecture = OutputArchitecture::AArch64;
                }
                else if (format == "rsi_nasm") {
                    outputFormat = OutputFormat::RSI_NASM;
                    outputArchitecture = OutputArchitecture::x86_64;
                }
                else if (format == "rsi_aarch64") {
                    outputFormat = OutputFormat::RSI_AArch64;
                    outputArchitecture = OutputArchitecture::AArch64;
                }
                else {
                    Error("Unknown output format \"" + format + "\"");
                    return static_cast<int>(ReturnValue::UnknownError);
                }
                i++;
            }
            else {
                Error("Missing output format");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else if (arg == "--compiler") {
            if (i + 1 < argc) {
                compiler = argv[++i];
            }
            else {
                Error("Missing compiler path");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else if (arg == "--stdlib") {
            if (i + 1 < argc) {
                stdlibIncludePath = argv[++i];
            }
            else {
                Error("Missing standard library path");
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
        else {
            // test if it is a filename
            if (std::filesystem::exists(arg)) {
                inputFilename = arg;
            }
            else {
                Error("Invalid argument: ", arg);
                return static_cast<int>(ReturnValue::UnknownError);
            }
        }
    }

    std::vector<Token> tokens;
    std::shared_ptr<AstProgram> ast;
    std::string outputSource;
    std::string R_Sharp_Source;

    Print("--------------| Tokenizing |--------------");
    {
        Tokenizer tokenizer(inputFilename);
        tokens = tokenizer.tokenize();
        R_Sharp_Source = tokenizer.getSource();

        for (auto const& token : tokens) {
            Print(token.toString());
        }

        tokens = cleanTokens(tokens);
    }

    Print("--------------| Parsing |--------------");
    {
        ParsingCache cache;
        Parser parser = Parser(tokens, inputFilename, stdlibIncludePath, cache);
        ast = parser.parse();

        Print("--------------| Syntax Errors |--------------");
        if (parser.hasErrors()) {
            ErrorPrinter printer(ast, inputFilename, R_Sharp_Source);
            printer.print();
            Error("Parsing errors.");
            return static_cast<int>(ReturnValue::SyntaxError);
        }
        else {
            Print("No errors");
        }
    }
    Print("--------------| Raw AST |--------------");
    {
        AstPrinter printer(ast);
        printer.print();
    }

    Print("--------------| Semantic Errors |--------------");
    {
        SemanticValidator validator(ast, inputFilename, R_Sharp_Source);
        validator.validate();

        if (validator.hasErrors()) {
            Error("Semantic errors.");
            return static_cast<int>(ReturnValue::SemanticError);
        }
        else {
            Print("No errors");
        }
    }
    Print("--------------| Typed AST |--------------");
    {
        AstPrinter printer(ast);
        printer.print();
    }


    RSI::TranslationUnit translationUnit;
    Print("--------------| Generated code |--------------");
    {
        switch (outputFormat) {
            case OutputFormat::C:    outputSource = CCodeGenerator(ast).generate(); break;
            case OutputFormat::NASM: outputSource = NASMCodeGenerator(ast, R_Sharp_Source).generate(); break;
            case OutputFormat::AArch64:
                outputSource = AArch64CodeGenerator(ast, R_Sharp_Source).generate();
                break;
            case OutputFormat::RSI_NASM:
            case OutputFormat::RSI_AArch64:
                translationUnit = RSIGenerator(ast, R_Sharp_Source).generate();
                break;
        }
        if (outputSource.length())
            Print(outputSource);
        else {
            // clang-format off
            std::vector<RSIPass> passes = {
                RSIPass{
                    .humanHeader = "Raw RSI",
                    .architectures = allArchitectureTypes,
                    .positiveInstructionTypes = {RSI::InstructionType::NOP},
                    .perInstructionFunction = [](auto&, auto&, auto&){},
                },
                /*
                RSIPass{
                    .humanHeader = "Separeate global references",
                    .architectures = allArchitectureTypes,
                    .perInstructionFunction = RSI::separateGlobalReferences,
                },
                RSIPass{
                    .humanHeader = "Replace global reference with memory access",
                    .architectures = {OutputArchitecture::AArch64},
                    .positiveInstructionTypes = {RSI::InstructionType::MOVE},
                    .perInstructionFunction = RSI::globalReferenceToMemoryAccess,
                },
                */
                RSIPass{
                    .humanHeader = "Seperate divisions",
                    .architectures = {OutputArchitecture::x86_64},
                    .positiveInstructionTypes = {RSI::InstructionType::DIVIDE},
                    .perInstructionFunction = RSI::seperateDivReferences,
                },
                RSIPass{
                    .humanHeader = "Seperate calls",
                    .architectures = {OutputArchitecture::x86_64},
                    .positiveInstructionTypes = {RSI::InstructionType::CALL},
                    .perInstructionFunction = std::bind(
                        RSI::seperateCallResults,
                        x86_64,
                        std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3
                    ),
                },
                RSIPass{
                    .humanHeader = "Seperate calls",
                    .architectures = {OutputArchitecture::AArch64},
                    .positiveInstructionTypes = {RSI::InstructionType::CALL},
                    .perInstructionFunction = std::bind(
                        RSI::seperateCallResults,
                        aarch64,
                        std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3
                    ),
                },
                RSIPass{
                    .humanHeader = "Separate parameter loads",
                    .architectures = {OutputArchitecture::x86_64},
                    .positiveInstructionTypes = {RSI::InstructionType::LOAD_PARAMETER},
                    .perInstructionFunction = std::bind(
                        RSI::separateLoadParameters,
                        x86_64,
                        std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3
                    ),
                },
                RSIPass{
                    .humanHeader = "Separate parameter loads",
                    .architectures = {OutputArchitecture::AArch64},
                    .positiveInstructionTypes = {RSI::InstructionType::LOAD_PARAMETER},
                    .perInstructionFunction = std::bind(
                        RSI::separateLoadParameters,
                        aarch64,
                        std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3
                    ),
                },
                RSIPass{
                    .humanHeader = "Resolve addresses",
                    .architectures = {OutputArchitecture::AArch64},
                    .positiveInstructionTypes = {RSI::InstructionType::ADDRESS_OF},
                    .perInstructionFunction = std::bind(
                        RSI::resolveAddressOf,
                        aarch64,
                        std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3
                    ),
                },
                RSIPass{
                    .humanHeader = "Resolve addresses",
                    .architectures = {OutputArchitecture::x86_64},
                    .positiveInstructionTypes = {RSI::InstructionType::ADDRESS_OF},
                    .perInstructionFunction = std::bind(
                        RSI::resolveAddressOf,
                        x86_64,
                        std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3
                    ),
                },
                RSIPass{
                    .humanHeader = "Constants to references",
                    .architectures = allArchitectureTypes,
                    .negativeInstructionTypes = {RSI::InstructionType::MOVE, RSI::InstructionType::CALL, RSI::InstructionType::LOAD_PARAMETER},
                    .perInstructionFunction = RSI::moveConstantsToReferences,
                },
                RSIPass{
                    .humanHeader = "Two Operand Compatibility",
                    .architectures = {OutputArchitecture::x86_64},
                    .negativeInstructionTypes = {RSI::InstructionType::JUMP, RSI::InstructionType::JUMP_IF_ZERO, RSI::InstructionType::DEFINE_LABEL},
                    .prefilter = RSI::makeTwoOperandCompatible_prefilter,
                    .perInstructionFunction = RSI::makeTwoOperandCompatible,
                },
                RSIPass{
                    .humanHeader = "Replace modulo with div, mul, sub",
                    .architectures = {OutputArchitecture::AArch64},
                    .positiveInstructionTypes = {RSI::InstructionType::MODULO},
                    .perInstructionFunction = RSI::replaceModWithDivMulSub,
                },
                RSIPass{
                    .humanHeader = "Liveness analysis",
                    .architectures = allArchitectureTypes,
                    .positiveInstructionTypes = {},
                    .isFunctionWide = true,
                    .perFunctionFunction = RSI::analyzeLiveVariables,
                },
                RSIPass{
                    .humanHeader = "Sanity check",
                    .architectures = allArchitectureTypes,
                    .positiveInstructionTypes = {},
                    .isFunctionWide = true,
                    .perFunctionFunction = [](auto& func, auto){
                        if (func.instructions.at(0).meta.liveVariablesBefore.size() != 0) {
                            Fatal("Function \"", func.name, "\" requires live variables before main code. This probably means some transformation is incorrect.");
                        }
                    },
                },
                RSIPass{
                    .humanHeader = "Graph coloring register assignment",
                    .architectures = allArchitectureTypes,
                    .positiveInstructionTypes = {},
                    .isFunctionWide = true,
                    .perFunctionFunction = RSI::assignRegistersGraphColoring,
                },
                RSIPass{
                    .humanHeader = "Separate stack variables",
                    .architectures = {OutputArchitecture::AArch64},
                    .perInstructionFunction = RSI::separateStackVariables,
                },
                RSIPass{
                    .humanHeader = "Register enumeration",
                    .architectures = allArchitectureTypes,
                    .positiveInstructionTypes = {},
                    .isFunctionWide = true,
                    .perFunctionFunction = RSI::enumerateRegisters,
                },
            };
            // clang-format on

            for (auto& pass : passes) {
                pass(translationUnit.functions, outputArchitecture);
            }
            Print("--------------| RSI to assembly |--------------");
            if (outputArchitecture == OutputArchitecture::x86_64) {
                outputSource =
                    "; NASM code generated by R-Sharp compiler (using RSI)"
                    "\n"
                    "BITS 64\n"
                    "section .text\n"
                    "\n";

                for (auto label : translationUnit.externLabels) {
                    outputSource += "extern " + label->name + "\n";
                }
                for (auto& func : translationUnit.functions) {
                    outputSource += "global " + func.name + "\n";
                    outputSource += rsiToNasm(func) + "\n";
                }
                outputSource += "section .data\n";
                for (auto [ref, value] : translationUnit.initializedGlobalVariables) {
                    outputSource += ref->name + ": dq " + std::to_string(value.value) + "\n";
                }
                outputSource += "section .bss\n";
                for (auto ref : translationUnit.uninitializedGlobalVariables) {
                    outputSource += ref->name + ": resb 8\n";
                }
                outputFormat = OutputFormat::NASM;
            }
            else {
                outputSource =
                    "// Aarch64 code generated by R-Sharp compiler (using RSI)\n"
                    "\n"
                    ".macro push reg\n"
                    " str \\reg, [sp, -16]!\n"
                    ".endm\n"
                    ".macro pop reg\n"
                    " ldr \\reg, [sp], 16\n"
                    ".endm\n"
                    "\n"
                    ".text\n"
                    "\n";

                for (auto label : translationUnit.externLabels) {
                    outputSource += ".extern " + label->name + "\n";
                }
                for (auto& func : translationUnit.functions) {
                    outputSource += ".global " + func.name + "\n";
                    outputSource += rsiToAarch64(func) + "\n";
                }
                outputSource += ".data\n";
                for (auto [ref, value] : translationUnit.initializedGlobalVariables) {
                    outputSource += ref->name + ": .8byte " + std::to_string(value.value) + "\n";
                }
                outputSource += ".bss\n";
                for (auto ref : translationUnit.uninitializedGlobalVariables) {
                    outputSource += ref->name + ": .space 8\n";
                }
                outputFormat = OutputFormat::AArch64;
            }
            Print(outputSource);
        }
    }
    std::string temporaryFile = outputFilename;
    switch (outputFormat) {
        case OutputFormat::C:       temporaryFile += ".c"; break;
        case OutputFormat::NASM:    temporaryFile += ".asm"; break;
        case OutputFormat::AArch64: temporaryFile += ".S"; break;
        default:
            Error("Unknown output format");
            return static_cast<int>(ReturnValue::UnknownError);
            break;
    }

    Print("Writing to file: ", temporaryFile);
    std::ofstream outputFile(temporaryFile);
    if (outputFile.is_open()) {
        outputFile << outputSource;
        outputFile.close();
    }
    else {
        Error("Could not open file: ", temporaryFile);
        return static_cast<int>(ReturnValue::UnknownError);
    }

    std::string additionalyLinkedFiles_str;
    for (auto file : additionalyLinkedFiles) {
        additionalyLinkedFiles_str += file + " ";
    }

    const std::string gccArgumentsCompile =
        "-g -Werror -Wall -Wno-unused-variable -Wno-unused-value -Wno-unused-but-set-variable "
        "-Wno-unused-function";
    const std::string gccArgumentsLink = "-g -Werror -Wall -no-pie ";
    const std::string nasmArgumentsCompile = "-g -w+error";

    switch (outputFormat) {
        case OutputFormat::C: {
            Print("--------------| Compiling using gcc |--------------");
            std::string command = compiler + " " + gccArgumentsCompile + " " + temporaryFile + " "
                                + additionalyLinkedFiles_str + " -o " + outputFilename;
            Print("Executing: ", command);
            int success = !system(command.c_str());
            if (success)
                Print("Compilation successful.");
            else {
                Error("Compilation failed.");
                return static_cast<int>(ReturnValue::AssemblingError);
            }
            break;
        }
        case OutputFormat::AArch64: {
            Print("--------------| Compiling using gcc |--------------");
            std::string command = compiler + " " + gccArgumentsCompile + " " + temporaryFile + " "
                                + additionalyLinkedFiles_str + " -o " + outputFilename;
            Print("Executing: ", command);
            int success = !system(command.c_str());
            if (success)
                Print("Compilation successful.");
            else {
                Error("Compilation failed.");
                return static_cast<int>(ReturnValue::AssemblingError);
            }
            break;
        }
        case OutputFormat::NASM: {
            Print("--------------| Assembling using nasm |--------------");
            std::string command = "nasm " + nasmArgumentsCompile + " -f elf64 " + temporaryFile + " -o "
                                + outputFilename + ".o";
            Print("Executing: ", command);
            int success = !system(command.c_str());
            if (success)
                Print("Assembling successful.");
            else {
                Error("Assembling failed.");
                return static_cast<int>(ReturnValue::AssemblingError);
            }

            Print("--------------| Linking using gcc |--------------");
            command = compiler + " " + gccArgumentsLink + " " + outputFilename + ".o "
                    + additionalyLinkedFiles_str + " -o " + outputFilename;
            Print("Executing: ", command);
            success = !system(command.c_str());
            if (success)
                Print("Linking successful.");
            else {
                Error("Linking failed.");
                return static_cast<int>(ReturnValue::AssemblingError);
            }

            break;
        }
        default:
            Error("Unsupported output format");
            return static_cast<int>(ReturnValue::UnknownError);
            break;
    }

    return static_cast<int>(ReturnValue::NormalExit);
}
