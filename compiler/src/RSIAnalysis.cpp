#include "R-Sharp/RSIAnalysis.hpp"
#include "R-Sharp/RSIGenerator.hpp"
#include "R-Sharp/Graph.hpp"
#include "R-Sharp/Architecture.hpp"

#include "R-Sharp/Utils/ScopeGuard.hpp"


namespace RSI{

void analyzeLiveVariables(RSI::Function& function, OutputArchitecture){
    bool hasModifiedTheIR = true;

    const auto setSizeWatchGuard = [&hasModifiedTheIR](auto const& setToWatch){
        const auto sizeAtBeginning = setToWatch.size();
        return ScopeGuard([&setToWatch, sizeAtBeginning, &hasModifiedTheIR](){
            if (sizeAtBeginning != setToWatch.size()){
                hasModifiedTheIR = true;
            }
        });
    };

    auto virtualEndInstruction = std::make_unique<Instruction>(Instruction{
        .type = InstructionType::NOP,
    });
    while (hasModifiedTheIR){
        hasModifiedTheIR = false;

        Instruction* lastInstruction = virtualEndInstruction.get();
        for (auto it = function.instructions.rbegin(); it != function.instructions.rend(); it++){
            auto& instr = *it;
            const auto liveVariableBeforeThisPass = instr.meta.liveVariablesBefore;
            
            if (lastInstruction->meta.liveVariablesBefore.size() != 0){
                instr.meta.liveVariablesBefore.insert(lastInstruction->meta.liveVariablesBefore.begin(), lastInstruction->meta.liveVariablesBefore.end());
            }

            if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.result)){
                instr.meta.liveVariablesBefore.erase(std::get<std::shared_ptr<RSI::Reference>>(instr.result));
            }

            if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.op1)) {
                instr.meta.liveVariablesBefore.insert(std::get<std::shared_ptr<RSI::Reference>>(instr.op1));
            }
            if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(instr.op2)) {
                instr.meta.liveVariablesBefore.insert(std::get<std::shared_ptr<RSI::Reference>>(instr.op2));
            }

            if (instr.type == RSI::InstructionType::DEFINE_LABEL){
                for (auto& jump_instr : function.instructions){
                    if (jump_instr.type != RSI::InstructionType::JUMP && jump_instr.type != RSI::InstructionType::JUMP_IF_ZERO)
                        continue;

                    if (jump_instr.op1 == instr.op1 || jump_instr.op2 == instr.op1){
                        const auto guard = setSizeWatchGuard(jump_instr.meta.liveVariablesBefore);
                        jump_instr.meta.liveVariablesBefore.insert(instr.meta.liveVariablesBefore.begin(), instr.meta.liveVariablesBefore.end());
                    }
                }
            }

            if (liveVariableBeforeThisPass != instr.meta.liveVariablesBefore){
                hasModifiedTheIR = true;
            }

            lastInstruction = &*it;
        }
        Print("LVA Pass");
    }
}

/*
Only works for strictly linear programs without any jumps. Use *assignRegistersGraphColoring* for everything else.
*/
void assignRegistersLinearScan(Function& func, OutputArchitecture arch){
    std::vector<HWRegister> const& allRegisters = arch == OutputArchitecture::x86_64 ? x86_64Registers : aarch64Registers;
    
    for (auto& instr : func.instructions){
        std::vector<HWRegister> unusedRegisters = allRegisters;
        for (auto& var : instr.meta.liveVariablesBefore){
            if (var->assignedRegister.has_value()){
                unusedRegisters.erase(std::remove(unusedRegisters.begin(), unusedRegisters.end(), var->assignedRegister.value()), unusedRegisters.end());
            }
        }

        for (auto& var : instr.meta.liveVariablesBefore){
            if (!var->assignedRegister.has_value()){
                var->assignedRegister = unusedRegisters.front();
                unusedRegisters.erase(unusedRegisters.begin());
            }
        }
    }
}

void assignRegistersGraphColoring(Function& func, OutputArchitecture arch){
    std::vector<HWRegister> const& allRegisters = arch == OutputArchitecture::x86_64 ? x86_64Registers : aarch64Registers;

    Graph<void> interferenceGraph;
    using Vertex = Vertex<void>;
    using NList = Vertex::NeighbourList;

    std::map<std::shared_ptr<RSI::Reference>, std::shared_ptr<Vertex>> referenceToVertex;
    std::map<std::shared_ptr<Vertex>, std::shared_ptr<RSI::Reference>> vertexToReference;

    const auto addVertexToGraph = [&](auto& operand){
        if (std::holds_alternative<std::shared_ptr<RSI::Reference>>(operand)){
            std::shared_ptr<Vertex> vert = std::make_shared<Vertex>();
            auto ref = std::get<std::shared_ptr<RSI::Reference>>(operand);
            if (referenceToVertex.count(ref) == 0){
                referenceToVertex.insert_or_assign(ref, vert);
                vertexToReference.insert_or_assign(vert, ref);
                interferenceGraph.addVertex(vert);
            }
        }
    };

    std::optional<std::reference_wrapper<RSI::Instruction>> lastInstruction;
    for (auto& instr : func.instructions){
        addVertexToGraph(instr.result);
        addVertexToGraph(instr.op1);
        addVertexToGraph(instr.op2);

        if (lastInstruction.has_value() && std::holds_alternative<std::shared_ptr<RSI::Reference>>(lastInstruction.value().get().result)){
            for (auto liveVar : instr.meta.liveVariablesBefore){
                if (liveVar == std::get<std::shared_ptr<RSI::Reference>>(lastInstruction.value().get().result)) continue;

                interferenceGraph.addEdge(
                    referenceToVertex.at(liveVar),
                    referenceToVertex.at(std::get<std::shared_ptr<RSI::Reference>>(lastInstruction.value().get().result))
                );
            }
        }

        for (auto liveVar1 : instr.meta.liveVariablesBefore){
            for (auto liveVar2 : instr.meta.liveVariablesBefore){
                if (liveVar1 == liveVar2) continue;

                interferenceGraph.addEdge(
                    referenceToVertex.at(liveVar1),
                    referenceToVertex.at(liveVar2)
                );
            }
        }

        lastInstruction = instr;
    }

    auto allAvailableColors = VertexColor::getNColors(allRegisters.size());
    std::map<VertexColor, RSI::HWRegister> colorToHWRegister;
    for (int i=0; i<allRegisters.size(); i++){
        colorToHWRegister.insert({allAvailableColors.at(i), allRegisters.at(i)});
    }

    if (!interferenceGraph.colorIn(allAvailableColors)){
        Fatal("Unable to color graph with the given colors.");
    }

    for (auto vertex : interferenceGraph){
        vertexToReference.at(vertex)->assignedRegister = colorToHWRegister.at(vertex->color.value());
    }
}
bool makeTwoOperandCompatible_prefilter(RSI::Instruction const& instr){
    if (instr.type == InstructionType::NEGATE)
        return true;
    
    if (!std::holds_alternative<std::shared_ptr<Reference>>(instr.result) || !std::holds_alternative<std::shared_ptr<Reference>>(instr.op1) || std::holds_alternative<std::monostate>(instr.op2))
        return false;
    
    return instr.result != instr.op1;
}

void makeTwoOperandCompatible(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions){
    Instruction move{
        .type = InstructionType::MOVE,
        .result = instr.result,
        .op1 = instr.op1,
    };
    instr.op1 = instr.result;
    beforeInstructions.push_back(move);
}

void replaceModWithDivMulSub(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions){
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
void moveConstantsToReferences(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions){
    if (std::holds_alternative<RSI::Constant>(instr.op1)){
        RSI::Instruction move{
            .type = RSI::InstructionType::MOVE,
            .result = RSIGenerator::getNewReference("constant"),
            .op1 = instr.op1,
        };
        instr.op1 = move.result;

        beforeInstructions.push_back(move);
    }

    if (std::holds_alternative<RSI::Constant>(instr.op2)){
        RSI::Instruction move{
            .type = RSI::InstructionType::MOVE,
            .result = RSIGenerator::getNewReference("constant"),
            .op1 = instr.op2,
        };
        instr.op2 = move.result;

        beforeInstructions.push_back(move);
    }
}

void seperateDivReferences(RSI::Instruction& instr, std::vector<RSI::Instruction>& beforeInstructions, std::vector<RSI::Instruction>& afterInstructions){
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

    std::get<std::shared_ptr<RSI::Reference>>(instr.op1)->assignedRegister = x86_64Registers.at(static_cast<int>(NasmRegisters::RAX));
    std::get<std::shared_ptr<RSI::Reference>>(instr.result)->assignedRegister = x86_64Registers.at(static_cast<int>(NasmRegisters::RAX));

    beforeInstructions.push_back(move1);
    beforeInstructions.push_back(move2);
    afterInstructions.push_back(moveRes);
}

}