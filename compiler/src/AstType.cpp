#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Logging.hpp"

AstType::AstType(RSharpType type){
    this->type = type;
}
AstType::AstType(RSharpType type, std::shared_ptr<AstType> subtype){
    this->type = type;
}

bool operator==(AstType const& a, AstType const& b){
    return a.type == b.type;
}
bool operator!=(AstType const& a, AstType const& b){
    return !(a == b);
}

RSharpType stringToType(std::string const& str){
    if(str == "i32"){
        return RSharpType::I32;
    }
    else if(str == "i64"){
        return RSharpType::I64;
    }
    return RSharpType::NONE;
}
std::string typeToString(RSharpType type){
    switch(type){
        case RSharpType::I32: return "i32";
        case RSharpType::I64: return "i64";
        case RSharpType::ErrorType: return "error type";
        case RSharpType::NONE: return "no type";

        default: return "undefined type";
    }
}