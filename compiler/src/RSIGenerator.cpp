#include "R-Sharp/RSIGenerator.hpp"
#include "R-Sharp/Logging.hpp"
#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Utils.hpp"
#include "R-Sharp/VariableSizeInserter.hpp"
#include "R-Sharp/Utils/ScopeGuard.hpp"

#include <sstream>
#include <map>

RSIGenerator::RSIGenerator(std::shared_ptr<AstProgram> root, std::string R_SharpSource){
    this->root = root;
    this->R_SharpSource = R_SharpSource;
}

int RSIGenerator::sizeFromSemanticalType(std::shared_ptr<AstType> type){
    static const std::map<RSharpPrimitiveType, int> primitive_sizes = {
        {RSharpPrimitiveType::C_void, 1}, // should only be used for pointer arithmetic
        {RSharpPrimitiveType::I8, 1},
        {RSharpPrimitiveType::I16, 2},
        {RSharpPrimitiveType::I32, 4},
        {RSharpPrimitiveType::I64, 8},
    };

    switch(type->getType()){
        case AstNodeType::AstPrimitiveType:{
            return primitive_sizes.at(std::static_pointer_cast<AstPrimitiveType>(type)->type);
        }
        case AstNodeType::AstPointerType:{
            return 8;
        }
        case AstNodeType::AstArrayType:{
            auto array = std::static_pointer_cast<AstArrayType>(type);
            if (!array->size.has_value()){
                throw std::runtime_error("Array without size during code generation.");
            }
            if (array->size.value()->getType() != AstNodeType::AstInteger){
                throw std::runtime_error("Array with non constant size during code generation.");
            }
            return sizeFromSemanticalType(array->subtype) * std::static_pointer_cast<AstInteger>(array->size.value())->value;
        }
        default: throw std::runtime_error("Unimplemented type used");
    }
}

std::vector<RSI::Function> RSIGenerator::generate(){
    arrayAccessFinalSize = 0;
    stackPassedValueSize = 0;

    expectedValueType = ValueType::Value;

    externalLabels = {"memset", "memcpy"};

    VariableSizeInserter sizeInserter(root);
    sizeInserter.insert(RSIGenerator::sizeFromSemanticalType);

    root->accept(this);

    return functions;
}

std::string RSIGenerator::makeStringUnique(std::string const& prefix){
    static uint64_t labelCounter = 0;
    return prefix + "_" + std::to_string(labelCounter++);
}
std::shared_ptr<RSI::Reference> RSIGenerator::getNewReference(std::string const& name){
    return std::make_shared<RSI::Reference>(RSI::Reference{.name = makeStringUnique(name)});
}
std::shared_ptr<RSI::Label> RSIGenerator::getNewLabel(std::string const& name){
    return std::make_shared<RSI::Label>(RSI::Label{.name = makeStringUnique(name)});
}

void RSIGenerator::setupLocalVariables(std::shared_ptr<AstBlock> scope){
    int max_name_length = 0;
    for (auto var : scope->variables){
        max_name_length = std::max<int>(max_name_length, var->name.length());
    }
    max_name_length += 4;
    // for (auto var : scope->variables){
    //     emitIndented("// " + var->name);
    //     for (int i=0; i<max_name_length - var->name.length(); i++) emit(" ");
    //     emit(std::to_string(std::get<int>(var->accessor)) + "\n");
    // }
}

// program
void RSIGenerator::visit(std::shared_ptr<AstProgram> node){
    node->globalScope->accept(this);
    
    for (auto var : node->globalScope->variables){
        var->accessor = makeStringUnique(var->name);
    }

    for (auto const& child : node->getChildren()){
        if (!child) continue;
        if (child->getType() == AstNodeType::AstFunctionDefinition){
            child->accept(this);
        }
        else if (child->getType() == AstNodeType::AstVariableDeclaration){
            child->accept(this);
        }
        else{
            Fatal("Invalid node type in program");
        }
    }


    // uninitialized global variables
    for (auto var : root->uninitializedGlobalVariables){
        if (!var->isDefined){
            Fatal("Not implemented!");
        }
    }
}
void RSIGenerator::visit(std::shared_ptr<AstParameterList> node){
    // generate access strings
    node->parameterBlock->accept(this);

    for (auto const& child : node->parameters){
        child->accept(this);
    }
}

// definitions
void RSIGenerator::visit(std::shared_ptr<AstFunctionDefinition> node){
    if(std::find(node->tags->tags.begin(), node->tags->tags.end(), AstTags::Value::Extern) == node->tags->tags.end()){
        auto& func = functions.emplace_back();
        func.name = node->functionData->name;
        func.function = node->functionData;

        emit(RSI::Instruction{
            .type = RSI::InstructionType::DEFINE_LABEL,
            .op1 = std::make_shared<RSI::Label>(RSI::Label{.name = func.name}),
        });

        node->parameters->accept(this);
        node->body->accept(this);

        emit(RSI::Instruction{
            .type = RSI::InstructionType::RETURN,
            .op1 = RSI::Constant{.value = 0},
        });
    }
    else{
        externalLabels.insert(node->functionData->name);
    }
}


// statements
void RSIGenerator::visit(std::shared_ptr<AstBlock> node){
    if (!node->isMerged) setupLocalVariables(node);


    for (auto child : node->getChildren()){
        if (child) child->accept(this);
    }

    // don't change stack pointer if it wasn't modified
    if (!node->isMerged) resetStackPointer(node);
}
void RSIGenerator::visit(std::shared_ptr<AstReturn> node){
    expectedValueType = ValueType::Value;
    node->value->accept(this);
    for (auto scope = node->containedScopes.rbegin(); scope != node->containedScopes.rend(); ++scope){
        if (scope->expired()){
            Fatal("INTERNAL ERROR: std::weak_ptr expired during code generation.");
        }
        else{
            resetStackPointer(scope->lock());
        }
    }
    emit(RSI::Instruction{
        .type = RSI::InstructionType::RETURN,
        .op1 = lastResult,
    });
}
void RSIGenerator::visit(std::shared_ptr<AstConditionalStatement> node){
    const auto else_label = getNewLabel(".else");
    const auto end_label = getNewLabel(".end");

    node->condition->accept(this);
    // emitIndented("// If statement\n");
    emit(RSI::Instruction{
        .type = RSI::InstructionType::JUMP_IF_ZERO,
        .op1 = lastResult,
        .op2 = else_label,
    });
    node->trueStatement->accept(this);
    emit(RSI::Instruction{
        .type = RSI::InstructionType::JUMP,
        .op1 = end_label,
    });
    emit(RSI::Instruction{
        .type = RSI::InstructionType::DEFINE_LABEL,
        .op1 = else_label,
    });
    if (node->falseStatement) node->falseStatement->accept(this);
    emit(RSI::Instruction{
        .type = RSI::InstructionType::DEFINE_LABEL,
        .op1 = end_label,
    });
}
void RSIGenerator::visit(std::shared_ptr<AstForLoopDeclaration> node){
    auto const start_label = getNewLabel(".start");
    auto const end_label = getNewLabel(".end");
    auto const increment_label = getNewLabel(".increment");

    node->loop->skipLabel = increment_label;
    node->loop->breakLabel = end_label;

    // setupLocalVariables(node->initializationContext);
    node->initialization->accept(this);

    emit(RSI::Instruction{
        .type = RSI::InstructionType::DEFINE_LABEL,
        .op1 = start_label,
    });

    node->condition->accept(this);
    emit(RSI::Instruction{
        .type = RSI::InstructionType::JUMP_IF_ZERO,
        .op1 = lastResult,
        .op2 = end_label,
    });
    node->body->accept(this);
    
    emit(RSI::Instruction{
        .type = RSI::InstructionType::DEFINE_LABEL,
        .op1 = increment_label,
    });
    node->increment->accept(this);
    emit(RSI::Instruction{
        .type = RSI::InstructionType::JUMP,
        .op1 = start_label,
    });
    emit(RSI::Instruction{
        .type = RSI::InstructionType::DEFINE_LABEL,
        .op1 = end_label,
    });

    // // manually restore the stack pointer
    // resetStackPointer(node->initializationContext);
}
void RSIGenerator::visit(std::shared_ptr<AstForLoopExpression> node){
    auto const start_label = getNewLabel(".start");
    auto const end_label = getNewLabel(".end");
    auto const increment_label = getNewLabel(".increment");

    node->loop->skipLabel = increment_label;
    node->loop->breakLabel = end_label;

    node->variable->accept(this);

    emit(RSI::Instruction{
        .type = RSI::InstructionType::DEFINE_LABEL,
        .op1 = start_label,
    });

    node->condition->accept(this);
    emit(RSI::Instruction{
        .type = RSI::InstructionType::JUMP_IF_ZERO,
        .op1 = lastResult,
        .op2 = end_label,
    });
    node->body->accept(this);
    
    emit(RSI::Instruction{
        .type = RSI::InstructionType::DEFINE_LABEL,
        .op1 = increment_label,
    });
    node->increment->accept(this);
    emit(RSI::Instruction{
        .type = RSI::InstructionType::JUMP,
        .op1 = start_label,
    });
    emit(RSI::Instruction{
        .type = RSI::InstructionType::DEFINE_LABEL,
        .op1 = end_label,
    });
}
void RSIGenerator::visit(std::shared_ptr<AstWhileLoop> node){
    std::string start_label = "." + makeStringUnique("start");
    std::string end_label = "." + makeStringUnique("end");

    node->loop->skipLabel = start_label;
    node->loop->breakLabel = end_label;

    Fatal("Not implemented!");
    // emitIndented("// While loop\n");
    // emitIndented(start_label + ":\n");
    // indent();
    // node->condition->accept(this);
    // emitIndented("cbz x0, " + end_label + "\n");
    // emitIndented("// Body\n");
    // node->body->accept(this);
    // emitIndented("b " + start_label + "\n");
    // dedent();
    // emitIndented(end_label + ":\n");
}
void RSIGenerator::visit(std::shared_ptr<AstDoWhileLoop> node){
    std::string start_label = "." + makeStringUnique("start");
    std::string end_label = "." + makeStringUnique("end");

    node->loop->skipLabel = start_label;
    node->loop->breakLabel = end_label;

    Fatal("Not implemented!");
    // emitIndented("// Do loop\n");
    // emitIndented(start_label + ":\n");
    // indent();
    // emitIndented("// Body\n");
    // node->body->accept(this);
    // node->condition->accept(this);
    // emitIndented("cbnz x0, " + start_label + "\n");
    // dedent();
    // emitIndented(end_label + ":\n");

}
void RSIGenerator::visit(std::shared_ptr<AstBreak> node){
    for (auto varScope = node->containedScopes.rbegin(); varScope != node->containedScopes.rend(); ++varScope){
        if (varScope->expired()){
            Fatal("INTERNAL ERROR: std::weak_ptr expired during code generation.");
        }
        else{
            resetStackPointer(varScope->lock());
        }
    }

    emit(RSI::Instruction{
        .type = RSI::InstructionType::JUMP,
        .op1 = std::get<std::shared_ptr<RSI::Label>>(node->loop->breakLabel),
    });
}
void RSIGenerator::visit(std::shared_ptr<AstSkip> node){
    for (auto varScope = node->containedScopes.rbegin(); varScope != node->containedScopes.rend(); ++varScope){
        if (varScope->expired()){
            Fatal("INTERNAL ERROR: std::weak_ptr expired during code generation.");
        }
        else{
            resetStackPointer(varScope->lock());
        }
    }

    emit(RSI::Instruction{
        .type = RSI::InstructionType::JUMP,
        .op1 = std::get<std::shared_ptr<RSI::Label>>(node->loop->skipLabel),
    });
}


// expressions
void RSIGenerator::visit(std::shared_ptr<AstUnary> node){
    expectValueType(ValueType::Value);
    node->value->accept(this);
    
    RSI::Instruction instr{
        .result = getNewReference(),
        .op1 = lastResult,
    };

    switch (node->type){
        case AstUnaryType::Negate:
            instr.type = RSI::InstructionType::NEGATE;
            break;
        case AstUnaryType::BinaryNot:
            instr.type = RSI::InstructionType::BINARY_NOT;
            break;
        case AstUnaryType::LogicalNot:
            instr.type = RSI::InstructionType::LOGICAL_NOT;
            break;
        default:
            Error("RSI Generator: Unary operator not implemented!");
            printErrorToken(node->token, R_SharpSource);
            exit(1);
            break;
    }
    emit(instr);
}
void RSIGenerator::visit(std::shared_ptr<AstBinary> node){
    node->left->accept(this);
    const auto leftValue = lastResult;

    // logical and and or will short circuit, so the right side is not evaluated until necessary
    if (node->type == AstBinaryType::LogicalOr){
        auto end_label = getNewLabel(".logical_or_end");
        auto right_label = getNewLabel(".logical_or_right");
        auto result = getNewReference("result");
        
        emit(RSI::Instruction{
            .type = RSI::InstructionType::JUMP_IF_ZERO,
            .op1 = leftValue,
            .op2 = right_label,
        });
        emit(RSI::Instruction{
            .type = RSI::InstructionType::MOVE,
            .result = result,
            .op1 = RSI::Constant{.value = 1},
        });
        emit(RSI::Instruction{
            .type = RSI::InstructionType::JUMP,
            .op1 = end_label,
        });


        emit(RSI::Instruction{
            .type = RSI::InstructionType::DEFINE_LABEL,
            .op1 = right_label,
        });
        node->right->accept(this);
        auto rightValue = lastResult;
        emit(RSI::Instruction{
            .type = RSI::InstructionType::NOT_EQUAL,
            .result = result,
            .op1 = rightValue,
            .op2 = RSI::Constant{.value = 0},
        });

        emit(RSI::Instruction{
            .type = RSI::InstructionType::DEFINE_LABEL,
            .op1 = end_label,
        });

        lastResult = result;
        return;
    }
    else if (node->type == AstBinaryType::LogicalAnd){
        auto end_label = getNewLabel(".logical_and_end");
        auto right_label = getNewLabel(".logical_and_right");
        auto result = getNewReference();
    
        auto isZeroRef = getNewReference();
        emit(RSI::Instruction{
            .type = RSI::InstructionType::EQUAL,
            .result = isZeroRef,
            .op1 = leftValue,
            .op2 = RSI::Constant{.value = 0},
        });
        emit(RSI::Instruction{
            .type = RSI::InstructionType::JUMP_IF_ZERO,
            .op1 = isZeroRef,
            .op2 = right_label,

        });
        emit(RSI::Instruction{
            .type = RSI::InstructionType::MOVE,
            .result = result,
            .op1 = RSI::Constant{.value = 0},
        });
        emit(RSI::Instruction{
            .type = RSI::InstructionType::JUMP,
            .op1 = end_label,
        });


        emit(RSI::Instruction{
            .type = RSI::InstructionType::DEFINE_LABEL,
            .op1 = right_label,
        });
        node->right->accept(this);
        auto rightValue = lastResult;
        emit(RSI::Instruction{
            .type = RSI::InstructionType::NOT_EQUAL,
            .result = result,
            .op1 = rightValue,
            .op2 = RSI::Constant{.value = 0},
        });

        emit(RSI::Instruction{
            .type = RSI::InstructionType::DEFINE_LABEL,
            .op1 = end_label,
        });

        lastResult = result;
        return;
    }

    node->right->accept(this);

    RSI::Instruction instr{
        .result = getNewReference(),
        .op1 = leftValue,
        .op2 = lastResult,
    };
    

    switch (node->type){
        case AstBinaryType::Add:
            if (node->left->semanticType->getType() == AstNodeType::AstPointerType){
                Fatal("Not implemented!");
                // emitIndented("mov x2, " + std::to_string(sizeFromSemanticalType(std::static_pointer_cast<AstPointerType>(node->left->semanticType)->subtype)) + "\n");
                // // x0 = x0 + (x1 * x2)
                // emitIndented("madd x0, x1, x2, x0\n");
            }
            else if (node->right->semanticType->getType() == AstNodeType::AstPointerType){
                Fatal("Not implemented!");
                // emitIndented("mov x2, " + std::to_string(sizeFromSemanticalType(std::static_pointer_cast<AstPointerType>(node->right->semanticType)->subtype)) + "\n");
                // // x0 = x1 + (x0 * x2)
                // emitIndented("madd x0, x0, x2, x1\n");
            }
            else{
                expectValueType(ValueType::Value);
                instr.type = RSI::InstructionType::ADD;
            }
            break;
        case AstBinaryType::Subtract:
            if (node->left->semanticType->getType() == AstNodeType::AstPointerType){
                Fatal("Not implemented!");
                // emitIndented("mov x2, " + std::to_string(sizeFromSemanticalType(node->left->semanticType)) + "\n");
                // // x0 = x0 - (x1 * x2)
                // emitIndented("msub x0, x1, x2, x0\n");
            }
            else if (node->right->semanticType->getType() == AstNodeType::AstPointerType){
                Fatal("Not implemented!");
                // emitIndented("mov x2, " + std::to_string(sizeFromSemanticalType(node->left->semanticType)) + "\n");
                // // x0 = x1 - (x0 * x2)
                // emitIndented("msub x0, x0, x2, x1\n");
            }
            else{
                expectValueType(ValueType::Value);
                instr.type = RSI::InstructionType::SUBTRACT;
            }
            break;
        case AstBinaryType::Multiply:
            expectValueType(ValueType::Value);
            instr.type = RSI::InstructionType::MULTIPLY;
            break;
        case AstBinaryType::Divide:
            expectValueType(ValueType::Value);
            instr.type = RSI::InstructionType::DIVIDE;
            break;
        case AstBinaryType::Modulo:
            expectValueType(ValueType::Value);
            instr.type = RSI::InstructionType::MODULO;
            break;

        case AstBinaryType::Equal:
            expectValueType(ValueType::Value);
            instr.type = RSI::InstructionType::EQUAL;
            break;
        case AstBinaryType::NotEqual:
            expectValueType(ValueType::Value);
            instr.type = RSI::InstructionType::NOT_EQUAL;
            break;
        case AstBinaryType::LessThan:
            expectValueType(ValueType::Value);
            instr.type = RSI::InstructionType::LESS_THAN;
            break;
        case AstBinaryType::LessThanOrEqual:
            expectValueType(ValueType::Value);
            instr.type = RSI::InstructionType::LESS_THAN_OR_EQUAL;
            break;
        case AstBinaryType::GreaterThan:
            expectValueType(ValueType::Value);
            instr.type = RSI::InstructionType::GREATER_THAN;
            break;
        case AstBinaryType::GreaterThanOrEqual:
            expectValueType(ValueType::Value);
            instr.type = RSI::InstructionType::GREATER_THAN_OR_EQUAL;
            break;

        case AstBinaryType::LogicalAnd:{
            expectValueType(ValueType::Value);
            Fatal("Not implemented!");
            // emitIndented("// Logical And\n");
            // std::string end = "." + getUniqueLabel("end");
            // emitIndented("cbz x0, " + end + "\n");

            // // evaluate right side
            // node->right->accept(this);
            // emitIndented("cmp x0, 0\n");
            // emitIndented("cset x0, ne\n");
            // emitIndented(end + ":\n");
            break;
        }

        case AstBinaryType::LogicalOr:{
            expectValueType(ValueType::Value);
            Fatal("Not implemented!");
            // emitIndented("// Logical Or\n");
            // std::string clause2 = "." + getUniqueLabel("second_expression");
            // std::string end = "." + getUniqueLabel("end");
            // emitIndented("cbz x0, " + clause2 + "\n");
            // emitIndented("mov x0, 1\n");
            // emitIndented("b " + end + "\n");
            // emitIndented(clause2 + ":\n"); indent();

            // // evaluate right side
            // node->right->accept(this);
            // emitIndented("cmp x0, 0\n");
            // emitIndented("cset x0, ne\n");
            // dedent();
            // emitIndented(end + ":\n");
            break;
        }
        default:
            Error("RSI Generator: Binary operator not implemented!");
            printErrorToken(node->token, R_SharpSource);
            exit(1);
            break;
    }

    emit(instr);
}
void RSIGenerator::visit(std::shared_ptr<AstInteger> node){
    expectValueType(ValueType::Value);

    lastResult = RSI::Constant{.value = static_cast<uint64_t>(node->value)};
}
void RSIGenerator::visit(std::shared_ptr<AstVariableAccess> node){
    auto size = sizeFromSemanticalType(node->semanticType);
    if (node->semanticType->getType() == AstNodeType::AstArrayType && expectedValueType == ValueType::Value){
        Fatal("Not implemented!");
        // emitIndented("// Copy " + std::to_string(size) + " bytes of stack space\n");
        // emitIndented("sub sp, sp, " + std::to_string(size) + "\n");
        // emitIndented("mov x0, sp\n");
        // if (node->variable->isGlobal)
        //     emitIndented("ldr x1, =[" + std::get<std::string>(node->variable->accessor) + "]\n");
        // else
        //     emitIndented("sub x1, fp, " + std::to_string(std::get<int>(node->variable->accessor)) + "\n");
        // emitIndented("mov x2, " + std::to_string(size) + "\n");
        // functionCallPrologue();
        // emitIndented("bl memcpy\n");
        // functionCallEpilogue();
    }
    else{
        lastResult = node->variable->rsiReference;
    }
}
void RSIGenerator::visit(std::shared_ptr<AstAssignment> node){
    expectValueType(ValueType::Value);
    expectedValueType = ValueType::Value;
    node->rvalue->accept(this);
    auto rvalue = lastResult;

    expectedValueType = ValueType::Address;
    node->lvalue->accept(this);
    expectedValueType = ValueType::Value;
    auto lvalue = lastResult;

    if (node->lvalue->semanticType->getType() == AstNodeType::AstArrayType){
        Fatal("Not implemented!");
        // emitIndented("mov x1, sp\n");
        // emitIndented("ldr x2, =" + std::to_string(stackPassedValueSize) + "\n");
        // functionCallPrologue();
        // emitIndented("bl memcpy\n");
        // functionCallEpilogue();
    }
    else {
        emit(RSI::Instruction{
            .type = RSI::InstructionType::MOVE,
            .result = lvalue,
            .op1 = rvalue,
        });
    }
}
void RSIGenerator::visit(std::shared_ptr<AstConditionalExpression> node){
    expectValueType(ValueType::Value);
    const auto false_clause = getNewLabel(".false_expression");
    const auto end_label = getNewLabel(".end");

    const auto result = getNewReference();
    
    node->condition->accept(this);
    // emitIndented("// If statement\n");
    emit(RSI::Instruction{
        .type = RSI::InstructionType::JUMP_IF_ZERO,
        .op1 = lastResult,
        .op2 = false_clause,
    });
    node->trueExpression->accept(this);
    emit(RSI::Instruction{
        .type = RSI::InstructionType::MOVE,
        .result = result,
        .op1 = lastResult,
    });
    emit(RSI::Instruction{
        .type = RSI::InstructionType::JUMP,
        .op1 = end_label,
    });
    emit(RSI::Instruction{
        .type = RSI::InstructionType::DEFINE_LABEL,
        .op1 = false_clause,
    });
    node->falseExpression->accept(this);
    emit(RSI::Instruction{
        .type = RSI::InstructionType::MOVE,
        .result = result,
        .op1 = lastResult,
    });
    emit(RSI::Instruction{
        .type = RSI::InstructionType::DEFINE_LABEL,
        .op1 = end_label,
    });

    lastResult = result;
}
void RSIGenerator::visit(std::shared_ptr<AstEmptyExpression> node){
    expectValueType(ValueType::Value);
    lastResult = RSI::Constant{.value = 1};
}
void RSIGenerator::visit(std::shared_ptr<AstExpressionStatement> node){
    stackPassedValueSize = 0;
    expectedValueType = ValueType::Value;
    node->expression->accept(this);
    if (node->expression->semanticType->getType() == AstNodeType::AstArrayType){
        Fatal("Not implemented!");
        // emitIndented("// Cleanup after array expression\n");
        // emitIndented("add sp, sp, " + std::to_string(stackPassedValueSize) + "\n");
    }
}

void RSIGenerator::visit(std::shared_ptr<AstFunctionCall> node){
    expectValueType(ValueType::Value);
    Fatal("Not implemented!");
    // // evaluate arguments
    // for (auto arg : node->arguments){
    //     expectedValueType = ValueType::Value;
    //     arg->accept(this);
    //     emitIndented("push x0\n");
    // }
    // if (node->arguments.size() > 8){
    //     Error("RSI Generator: More than 8 function arguments aren't yet supported!");
    //     printErrorToken(node->arguments.at(8)->token, R_SharpSource);
    //     exit(1);
    // }
    // // move arguments to registers
    // for (int i=node->arguments.size()-1; i>=0; i--){
    //     emitIndented("pop x" + std::to_string(i) + "\n");
    // }

    // emitIndented("// Prepare for function call (" + node->name + ")\n");
    // functionCallPrologue();
    // emitIndented("// Function Call (" + node->name + ")\n");
    // emitIndented("bl " + node->function->name + "\n");

    // emitIndented("// Restore after function call (" + node->name + ")\n");
    // functionCallEpilogue();
}
void RSIGenerator::visit(std::shared_ptr<AstAddressOf> node){
    expectValueType(ValueType::Value);
    Fatal("Not implemented!");
    // expectedValueType = ValueType::Address;
    // node->operand->accept(this);
    // expectedValueType = ValueType::Value;
}
void RSIGenerator::visit(std::shared_ptr<AstTypeConversion> node){
    int originSize = sizeFromSemanticalType(node->value->semanticType);
    int targetSize = sizeFromSemanticalType(node->semanticType);
    node->value->accept(this);
    if (expectedValueType == ValueType::Value){
        // only typecast values, no addresses
        Warning("Not using type cast \"sanity and\". May produce wrong outputs.");
        // emit(RSI::Instruction{
        //     .type = RSI::InstructionType::BINARY_AND,
        //     .result = RSI::Reference{
        //         .name = getUniqueLabel("tmp"),
        //     },
        //     .op1 = lastResult,
        //     .op2 = RSI::Constant{
        //         .value = uint64_t((__uint128_t(1) << targetSize*8)-1),
        //     }
        // });

        // if (targetSize > originSize){
        //     emitIndented("// Convert from " + std::to_string(originSize) + " bytes to " + std::to_string(targetSize) + " bytes.\n");
        //     emitIndented("sxt" + sizeToSuffix(originSize, true) + " " + getRegisterWithSize(0, targetSize) + ", w0\n");
        // }
        // else if (targetSize == originSize);
        // else{
        //     emitIndented("// explicit and to detect invalid upcasts later (" + std::to_string(originSize) + " Bytes --> " + std::to_string(targetSize) + " Bytes)\n");
        //     emitIndented("mov x1, " + std::to_string(uint64_t((__uint128_t(1) << __uint128_t(targetSize*8))-1)) + "\n");
        //     emitIndented("and x0, x0, x1\n");
        // }
    }
}



// declarations
void RSIGenerator::visit(std::shared_ptr<AstVariableDeclaration> node){
    expectedValueType = ValueType::Value;
    if (node->variable->isGlobal){
        if (!node->value){
            return;
        }
        Fatal("Not implemented!");
        // emitIndented("// Global Variable (" + node->name + ")\n", BinarySection::Data);
        // emitIndented(".global " + std::get<std::string>(node->variable->accessor) + "\n", BinarySection::Data);
        // emitIndented(std::get<std::string>(node->variable->accessor) + ":\n", BinarySection::Data);
        // indent(BinarySection::Data);
        // defineGlobalData(node->value);
        // dedent(BinarySection::Data);
    }
    else{
        if (node->variable->type.lock()->getType() == AstNodeType::AstArrayType){
            Fatal("Not implemented!");
            // if (node->value){
            //     node->value->accept(this);
            //     auto size = node->variable->sizeInBytes;
            //     emitIndented("// Copy " + std::to_string(size) + " bytes of stack space\n");
            //     if (node->variable->isGlobal)
            //         emitIndented("ldr x0, =[" + std::get<std::string>(node->variable->accessor) + "]\n");
            //     else
            //         emitIndented("sub x0, fp, " + std::to_string(std::get<int>(node->variable->accessor)) + "\n");
            //     emitIndented("mov x1, sp\n");
            //     emitIndented("ldr x2, =" + std::to_string(size) + "\n");
            //     functionCallPrologue();
            //     emitIndented("bl memcpy\n");
            //     functionCallEpilogue();
            //     emitIndented("add sp, sp, " + std::to_string(size) + "\n");
            // }
            // else{
            //     auto size = sizeFromSemanticalType(node->semanticType);
            //     emitIndented("// Zero out " + std::to_string(size) + " bytes of stack space\n");
            //     emitIndented("sub x0, fp, " + std::to_string(std::get<int>(node->variable->accessor)) + "\n");
            //     emitIndented("ldr x1, =0\n");
            //     emitIndented("ldr x2, =" + std::to_string(size) + "\n");
            //     functionCallPrologue();
            //     emitIndented("bl memset\n");
            //     functionCallEpilogue();
            // }
        }
        else{
            if (node->value){
                node->value->accept(this);
            }
            else{
                lastResult = RSI::Constant{
                    .value = 0,
                };
            }
            node->variable->rsiReference = getNewReference(node->variable->name);
            node->variable->rsiReference->variable = node->variable;
            emit(RSI::Instruction{
                .type = RSI::InstructionType::MOVE,
                .result = node->variable->rsiReference,
                .op1 = lastResult,
            });
        }
    }
}

void RSIGenerator::visit(std::shared_ptr<AstDereference> node){
    if (expectedValueType == ValueType::Value){
        int size = sizeFromSemanticalType(node->semanticType);
        node->operand->accept(this);
        Fatal("Not implemented!");
        // emitIndented("// Dereference\n");
        // emitIndented("ldr x0, [x0]\n");
        // emitIndented("// explicit and to detect invalid upcasts later (8 Bytes --> " + std::to_string(size) + " Bytes)\n");
        // emitIndented("mov x1, " + std::to_string(uint64_t((__uint128_t(1) << __uint128_t(size*8))-1)) + "\n");
        // emitIndented("and x0, x0, x1\n");
    }
    else{
        expectedValueType = ValueType::Value;
        node->operand->accept(this);
        // x0 already contains the address
    }
}
void RSIGenerator::visit(std::shared_ptr<AstArrayAccess> node){
    Fatal("Not implemented!");
    // emitIndented("// array access\n");
    // int targetSize = sizeFromSemanticalType(node->semanticType);

    // const auto prevArrayAccessFinalSize = arrayAccessFinalSize;
    // auto arrayAccessFinalSizeRestore = ScopeGuard([&](){
    //     arrayAccessFinalSize = prevArrayAccessFinalSize;
    // });

    // if (arrayAccessFinalSize == 0){
    //     emitIndented("mov x0, xzr\n");
    //     arrayAccessFinalSize = targetSize;
    // }

    // emitIndented("push x0\n");
    // const auto prevValueType = expectedValueType;
    // expectedValueType = ValueType::Value;
    // node->index->accept(this);
    // expectedValueType = prevValueType;
    // emitIndented("pop x1\n");
    // emitIndented("ldr x2, =" + std::to_string(targetSize) + "\n");
    // emitIndented("madd x0, x0, x2, x1\n");

    // if (node->array->getType() == AstNodeType::AstVariableAccess || node->array->getType() == AstNodeType::AstDereference){
    //     emitIndented("mov x2, x0\n");
    //     const auto prevValueType = expectedValueType;
    //     expectedValueType = ValueType::Address;
    //     node->array->accept(this);
    //     expectedValueType = prevValueType;

    //     emitIndented("add x0, x0, x2\n");
    //     if (expectedValueType == ValueType::Value)
    //         emitIndented("ldr" + sizeToSuffix(arrayAccessFinalSize) + " " + getRegisterWithSize(0, arrayAccessFinalSize) + ", [x0]\n");
    // }

    // else if (node->array->getType() == AstNodeType::AstArrayAccess){
    //     node->array->accept(this);
    // }
    // else if (node->array->getType() == AstNodeType::AstArrayLiteral){
    //     expectValueType(ValueType::Value);
    //     emitIndented("push x2\n");
    //     node->array->accept(this);
        
    //     // restore x2
    //     emitIndented("add x2, sp, " + std::to_string(stackPassedValueSize) + "\n");
    //     emitIndented("ldr x2, [x2]\n");

    //     emitIndented("add x2, sp, x2\n");
    //     emitIndented("ldr" + sizeToSuffix(arrayAccessFinalSize) + " " + getRegisterWithSize(0, arrayAccessFinalSize) + ", [x2]\n");

    //     // +16 for the pushed r2 and padding for 16-byte alignment
    //     emitIndented("add sp, sp, " + std::to_string(stackPassedValueSize+16) + "\n");
    // }
    // else{
    //     Error("NASM Generator: Unimplemented array access!");
    //     printErrorToken(node->token, R_SharpSource);
    //     exit(1);
    // }
}
void RSIGenerator::visit(std::shared_ptr<AstArrayLiteral> node) {
    Fatal("Not implemented!");
    // expectValueType(ValueType::Value);
    // uint64_t elementSize = sizeFromSemanticalType(std::dynamic_pointer_cast<AstArrayType>(node->semanticType)->subtype);
    // emitIndented("\n");
    // emitIndented("// Array Literal\n");
    // emitIndented("sub sp, sp, " + std::to_string(sizeFromSemanticalType(node->semanticType)) + "\n");
    // uint64_t currentOffset = 0;
    // for (auto element : node->elements){
    //     element->accept(this);
    //     emitIndented("add x19, sp, " + std::to_string(currentOffset) + "\n");
    //     switch(elementSize){
    //         case 1:
    //         case 2:
    //         case 4:
    //         case 8:
    //             emitIndented("str" + sizeToSuffix(elementSize) + " " + getRegisterWithSize(0, elementSize) + ", [x19]\n");
    //             break;
    //         default:{
    //             emitIndented("// Copy " + std::to_string(elementSize) + " bytes of stack space\n");
    //             emitIndented("add x0, sp, " + std::to_string(currentOffset+elementSize) + "\n");
    //             emitIndented("mov x1, sp\n");
    //             emitIndented("ldr x2, =" + std::to_string(elementSize) + "\n");
    //             functionCallPrologue();
    //             emitIndented("bl memcpy\n");
    //             functionCallEpilogue();
    //             break;
    //         }
    //     }
    //     currentOffset += elementSize;
    //     if (element->getType() == AstNodeType::AstArrayLiteral){
    //         emitIndented("add sp, sp, " + std::to_string(stackPassedValueSize) + "\n");
    //     }
    // }

    // stackPassedValueSize = sizeFromSemanticalType(node->semanticType);
}

void RSIGenerator::defineGlobalData(std::shared_ptr<AstExpression> node){
    Fatal("Not implemented!");
    // if (node->getType() == AstNodeType::AstInteger){
    //     auto intNode = std::dynamic_pointer_cast<AstInteger>(node);
    //     switch(sizeFromSemanticalType(node->semanticType)){
    //         case 1: emitIndented(".byte " + std::to_string(intNode->value) + "\n", BinarySection::Data); break;
    //         case 2: emitIndented(".2byte " + std::to_string(intNode->value) + "\n", BinarySection::Data); break;
    //         case 4: emitIndented(".4byte " + std::to_string(intNode->value) + "\n", BinarySection::Data); break;
    //         case 8: emitIndented(".8byte " + std::to_string(intNode->value) + "\n", BinarySection::Data); break;
    //         default:
    //             Error("RSI Generator: Global variable size not supported!");
    //             printErrorToken(node->token, R_SharpSource);
    //             exit(1);
    //             break;
    //     }
    // }
    // else if (node->getType() == AstNodeType::AstArrayLiteral){
    //     auto arrayNode = std::dynamic_pointer_cast<AstArrayLiteral>(node);
    //     for (auto element : arrayNode->elements){
    //         bool contains_array = element->semanticType->getType() == AstNodeType::AstArrayType;
    //         if (contains_array)
    //             indent(BinarySection::Data);

    //         defineGlobalData(element);
    //         if (contains_array){
    //             dedent(BinarySection::Data);
    //             emitIndented("\n", BinarySection::Data);
    //         }
    //     }
    // }
    // else{
    //     Error("RSI Generator: Global variable must be an integer or array. (Found: " + node->toString() + ")");
    //     printErrorToken(node->token, R_SharpSource);
    //     exit(1);
    // }
}