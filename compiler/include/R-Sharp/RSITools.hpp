#pragma once

#include "R-Sharp/RSI_FWD.hpp"

namespace RSI {

std::string stringify_operand(Operand const& op, std::map<HWRegister, std::string> const& registerTranslation);
std::string stringify_function(Function const& function, std::map<HWRegister, std::string> const& registerTranslation);

}