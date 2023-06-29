#include "R-Sharp/AstNodes.hpp"
#include "R-Sharp/Logging.hpp"

RSharpPrimitiveType stringToType(std::string const& str){
    if(str == "i8"){
        return RSharpPrimitiveType::I8;
    }
    else if(str == "i16"){
        return RSharpPrimitiveType::I16;
    }
    else if(str == "i32"){
        return RSharpPrimitiveType::I32;
    }
    else if(str == "i64"){
        return RSharpPrimitiveType::I64;
    }
    else if (str == "c_void"){
        return RSharpPrimitiveType::C_void;
    }
    return RSharpPrimitiveType::NONE;
}
std::string typeToString(RSharpPrimitiveType type){
    switch(type){
        case RSharpPrimitiveType::I8:  return "i8";
        case RSharpPrimitiveType::I16: return "i16";
        case RSharpPrimitiveType::I32: return "i32";
        case RSharpPrimitiveType::I64: return "i64";
        case RSharpPrimitiveType::C_void: return "c_void";
        case RSharpPrimitiveType::ErrorType: return "error type";
        case RSharpPrimitiveType::NONE: return "no type";

        default: return "undefined type";
    }
}