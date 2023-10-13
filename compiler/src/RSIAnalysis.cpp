#include "R-Sharp/RSIAnalysis.hpp"
#include "R-Sharp/RSIGenerator.hpp"
#include "R-Sharp/Graph.hpp"

#include "R-Sharp/Utils/LambdaOverload.hpp"
#include "R-Sharp/Utils/ScopeGuard.hpp"


namespace RSI{

std::string stringify_operand(Operand const& op, std::map<HWRegister, std::string> const& registerTranslation){
    return std::visit(lambda_overload{
        [](Constant const& x) { return std::to_string(x.value); },
        [&](std::shared_ptr<RSI::Reference> x){ return x->name + "(" + (x->assignedRegister.has_value() ? registerTranslation.at(x->assignedRegister.value()) : "None") + ")"; },
        [](std::shared_ptr<RSI::Label> x){ return x->name; },
        [](std::monostate const&){ Fatal("Empty RSI operand used!"); return std::string();},
    }, op);
}

std::string stringify_function(RSI::Function const& function, std::map<HWRegister, std::string> const& registerTranslation){
    const uint maxLiveVariableStringSize = 55;
    std::string result = "";
    for (auto instr : function.instructions){
        std::string prefix;
        prefix += "[";
        bool isFirst = true;
        for (auto liveVar : instr.meta.liveVariablesBefore){
            if (!isFirst){
                prefix += ", ";
            }
            isFirst = false;
            prefix += stringify_operand(liveVar, registerTranslation);
        }
        prefix += "]  ";
        while (prefix.length() < maxLiveVariableStringSize) prefix += " ";
        result += prefix;

        result += mnemonics.at(instr.type);
        
        if (instr.type == RSI::InstructionType::RETURN){
            result += " " + stringify_operand(instr.op1, registerTranslation) + "\n";
            continue;
        }
        else if (instr.type == RSI::InstructionType::DEFINE_LABEL){
            result += " " + stringify_operand(instr.op1, registerTranslation) + "\n";
            continue;
        }
        else if (instr.type == RSI::InstructionType::JUMP){
            result += " -> " + stringify_operand(instr.op1, registerTranslation) + "\n";
            continue;
        }
        else if (instr.type == RSI::InstructionType::JUMP_IF_ZERO){
            result += " " + stringify_operand(instr.op1, registerTranslation) + " -> " + stringify_operand(instr.op2, registerTranslation) + "\n";
            continue;
        }
        
        switch(numArgumentsUsed.at(instr.type)){
            case 0:
                result += "\n";
                break;
            case 1:
                result += " " + stringify_operand(instr.result, registerTranslation) + ", " + stringify_operand(instr.op1, registerTranslation) + "\n";
                break;
            case 2:
                result += " " + stringify_operand(instr.result, registerTranslation) + ", " + stringify_operand(instr.op1, registerTranslation) + ", " + stringify_operand(instr.op2, registerTranslation) + "\n";
                break;
            default:
                Fatal("Unimplemented number of arguments used in RSI.");
                break;
        }
    }
    return result;
}

void analyzeLiveVariables(RSI::Function& function){
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
void assignRegistersLinearScan(Function& func, std::vector<HWRegister> const& allRegisters){
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

void assignRegistersGraphColoring(Function& func, std::vector<HWRegister> const& allRegisters){
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

void makeTwoOperandCompatible(Function& func){
    for (int i=0; i<func.instructions.size(); i++){
        auto& instr = func.instructions.at(i);
        if (instr.type == InstructionType::JUMP || instr.type == InstructionType::JUMP_IF_ZERO || instr.type == InstructionType::DEFINE_LABEL){
            continue;
        }

        if (
            (std::holds_alternative<std::shared_ptr<Reference>>(instr.result) && std::holds_alternative<std::shared_ptr<Reference>>(instr.op1) && !std::holds_alternative<std::monostate>(instr.op2))
            || (instr.type == InstructionType::NEGATE)){
            if (instr.result != instr.op1){
                Instruction move{
                    .type = InstructionType::MOVE,
                    .result = instr.result,
                    .op1 = instr.op1,
                };
                instr.op1 = instr.result;
                func.instructions.insert(func.instructions.begin()+i, move);
            }
        }
    }
}

void replaceModWithDivMulSub(Function& func){
    for (int i=0; i<func.instructions.size(); i++){
        auto& instr = func.instructions.at(i);
        if (instr.type == InstructionType::MODULO){
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
            
            
            func.instructions.insert(func.instructions.begin()+i+1, mult);
            func.instructions.insert(func.instructions.begin()+i+2, sub);
        }
    }
}
void moveConstantsToReferences(Function& func){
    for (int i=0; i<func.instructions.size(); i++){
        int trackedI = i;
        if (func.instructions.at(trackedI).type == RSI::InstructionType::MOVE){
            continue;
        }

        if (std::holds_alternative<RSI::Constant>(func.instructions.at(trackedI).op1)){
            RSI::Instruction move{
                .type = RSI::InstructionType::MOVE,
                .result = RSIGenerator::getNewReference("constant"),
                .op1 = func.instructions.at(trackedI).op1,
            };
            func.instructions.at(trackedI).op1 = move.result;

            func.instructions.insert(func.instructions.begin()+trackedI, move);
            trackedI++;
        }

        if (std::holds_alternative<RSI::Constant>(func.instructions.at(trackedI).op2)){
            RSI::Instruction move{
                .type = RSI::InstructionType::MOVE,
                .result = RSIGenerator::getNewReference("constant"),
                .op1 = func.instructions.at(trackedI).op2,
            };
            func.instructions.at(trackedI).op2 = move.result;

            func.instructions.insert(func.instructions.begin()+trackedI, move);
            trackedI++;
        }
    }
}

void nasm_seperateDivReferences(Function& func, RSI::HWRegister rax){
    int i = -1;
    while (i+1 < func.instructions.size()){
        i++;
        if (func.instructions.at(i).type != RSI::InstructionType::DIVIDE){
            continue;
        }

        RSI::Instruction move1{
            .type = RSI::InstructionType::MOVE,
            .result = RSIGenerator::getNewReference("divtmp"),
            .op1 = func.instructions.at(i).op1,
        };
        RSI::Instruction move2{
            .type = RSI::InstructionType::MOVE,
            .result = RSIGenerator::getNewReference("divtmp"),
            .op1 = func.instructions.at(i).op2,
        };
        RSI::Instruction moveRes{
            .type = RSI::InstructionType::MOVE,
            .result = func.instructions.at(i).result,
            .op1 = RSIGenerator::getNewReference("divresult"),
        };
        func.instructions.at(i).op1 = move1.result;
        func.instructions.at(i).op2 = move2.result;
        func.instructions.at(i).result = moveRes.op1;

        std::get<std::shared_ptr<RSI::Reference>>(func.instructions.at(i).op1)->assignedRegister = rax;
        std::get<std::shared_ptr<RSI::Reference>>(func.instructions.at(i).result)->assignedRegister = rax;

        func.instructions.insert(func.instructions.begin()+i, move2); i++;
        func.instructions.insert(func.instructions.begin()+i, move1); i++;
        func.instructions.insert(func.instructions.begin()+i+1, moveRes);
    }
}

}