#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Logging.hpp"

AstType::AstType(RSharpType type){
    this->type = type;
}
AstType::AstType(RSharpType type, std::shared_ptr<AstType> subtype){
    this->type = type;
    this->subtype = subtype;
}

bool operator==(AstType const& a, AstType const& b){
    if(a.type != b.type){
        return false;
    }
    if (a.subtype && b.subtype && *a.subtype != *b.subtype){
        return false;
    }
    if (a.subtype || b.subtype){
        return false;
    }
    return true;
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

namespace std{
    string to_string(const AstType* type){
        std::string result;
        switch (type->type){
            case RSharpType::I32: result += "i32"; break;
            case RSharpType::I64: result += "i64"; break;
            case RSharpType::NONE: result += "none"; break;

            default:
                Fatal("Unimplemented type Nr. ", static_cast<int>(type->type));
                break;
        }


        return result;
    }
}