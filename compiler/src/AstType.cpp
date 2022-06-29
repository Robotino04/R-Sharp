#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Logging.hpp"

AstType::AstType(RSharpType type){
    this->type = type;
}
AstType::AstType(RSharpType type, std::shared_ptr<AstType> subtype){
    this->type = type;
    this->subtype = subtype;
}
AstType::AstType(RSharpType type, std::vector<RSharpModifier> modifier){
    this->type = type;
    this->modifiers = modifier;
}
AstType::AstType(RSharpType type, std::vector<RSharpModifier> modifier, std::shared_ptr<AstType> subtype){
    this->type = type;
    this->modifiers = modifier;
    this->subtype = subtype;
}

bool operator==(AstType const& a, AstType const& b){
    if(a.type != b.type){
        return false;
    }
    if(a.modifiers != b.modifiers){
        return false;
    }
    if (a.subtype && b.subtype){
        if(*a.subtype != *b.subtype){
            return false;
        }
    }
    if ((a.subtype && !b.subtype)
      || b.subtype && !a.subtype
      ){
        return false;
    }
    return true;
}
bool operator!=(AstType const& a, AstType const& b){
    return !(a == b);
}

RSharpModifier stringToModifier(std::string const& str){
    if(str == "const"){
        return RSharpModifier::CONST;
    }
    return RSharpModifier::NONE;
}
RSharpType stringToType(std::string const& str){
    if(str == "void"){
        return RSharpType::VOID;
    }
    if(str == "i32"){
        return RSharpType::I32;
    }
    if(str == "i64"){
        return RSharpType::I64;
    }
    return RSharpType::NONE;
}

namespace std{
    string to_string(const AstType* type){
        std::string result;
        
        for (auto modifier : type->modifiers){
            result += to_string(modifier) + " ";
        }

        
        if (type->type == RSharpType::ARRAY){
            result =  "[" + result + to_string(type->subtype.get()) + "]";
        }else if (type->type != RSharpType::NONE){
            switch (type->type){
                case RSharpType::I32: result += "i32"; break;
                case RSharpType::I64: result += "i64"; break;
                case RSharpType::VOID: result += "void"; break;

                default:
                    Fatal("Unimplemented type Nr. ", static_cast<int>(type->type));
                    break;
            }
        }


        return result;
    }
    string to_string(RSharpModifier mod){
        switch (mod){
            case RSharpModifier::CONST: return "const";
            case RSharpModifier::NONE: return "";

            default:
                Fatal("Unimplemented modifier Nr. ", static_cast<int>(mod));
                return "";
        }
    }
}