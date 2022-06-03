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
