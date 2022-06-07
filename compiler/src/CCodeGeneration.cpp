#include "R-Sharp/AstNodes.hpp"

void AstProgram::generateCCode(std::string& output){
    for (auto const& function : functions) {
        function->generateCCode(output);
        output += "\n";
    }
}

void AstFunction::generateCCode(std::string& output){
    returnType->generateCCode(output);
    output += " " + name;
    parameters->generateCCode(output);
    body->generateCCode(output);
}

void AstBlock::generateCCode(std::string& output){
    output += "{\n";
    for (auto const& statement : statements) {
        statement->generateCCode(output);
        output += "\n";
    }
    output += "}";
}

void AstReturn::generateCCode(std::string& output){
    output += "return ";
    value->generateCCode(output);
    output += ";";
}
void AstInteger::generateCCode(std::string& output){
    output += std::to_string(value);
}

void AstBuiltinType::generateCCode(std::string& output){
    for (auto const& modifier : modifiers) {
        modifier->generateCCode(output);
        output += " ";
    }
    output += name;
}

void AstArray::generateCCode(std::string& output){
    type->generateCCode(output);
    output += "*";
    for (auto const& modifier : modifiers) {
        modifier->generateCCode(output);
        output += " ";
    }
}

void AstTypeModifier::generateCCode(std::string& output){
    output += name;
}

void AstVariableDeclaration::generateCCode(std::string& output){
    type->generateCCode(output);
    output += " " + name;
    if (value){
        output += " = ";
        value->generateCCode(output);
    }
    output += ";";
}

void AstVariableAccess::generateCCode(std::string& output){
    output += name;
}

void AstVariableAssignment::generateCCode(std::string& output){
    output += "(" + name + " = ";
    value->generateCCode(output);
    output += ")";
}

void AstParameterList::generateCCode(std::string& output){
    output += "(";
    for (auto const& parameter : parameters) {
        parameter->generateCCode(output);
        if (parameter != parameters.back()) {
            output += ", ";
        }
    }
    output += ")";
}

void AstExpressionStatement::generateCCode(std::string& output){
    expression->generateCCode(output);
    output += ";";
}

void AstUnary::generateCCode(std::string& output){
    output += "(";
    switch (type) {
        case AstUnaryType::Negate:
            output += "-";
            break;
        case AstUnaryType::LogicalNot:
            output += "!";
            break;
        case AstUnaryType::BinaryNot:
            output += "~";
            break;
        default:
            Fatal("Invalid unary operator ", std::to_string(type));
            break;
    }
    value->generateCCode(output);
    output += ")";
}

void AstBinary::generateCCode(std::string& output){
    output += "(";
    left->generateCCode(output);
    switch (type){
        case AstBinaryType::Add:
            output += "+";
            break;
        case AstBinaryType::Subtract:
            output += "-";
            break;
        case AstBinaryType::Multiply:
            output += "*";
            break;
        case AstBinaryType::Divide:
            output += "/";
            break;

        case AstBinaryType::Equal:
            output += "==";
            break;
        case AstBinaryType::NotEqual:
            output += "!=";
            break;
        case AstBinaryType::LessThan:
            output += "<";
            break;
        case AstBinaryType::LessThanOrEqual:
            output += "<=";
            break;
        case AstBinaryType::GreaterThan:
            output += ">";
            break;
        case AstBinaryType::GreaterThanOrEqual:
            output += ">=";
            break;

        case AstBinaryType::LogicalAnd:
            output += "&&";
            break;
        case AstBinaryType::LogicalOr:
            output += "||";
            break;

        default:
            Fatal("Invalid binary operator ", std::to_string(type));
            break;
    }
    right->generateCCode(output);
    output += ")";
}