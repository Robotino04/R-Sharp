#pragma once

#include <string>
#include "R-Sharp/RSI.hpp"

namespace RSI{

std::string stringify_rsi_operand(RSIOperand const& op);

std::string stringify_rsi(RSIFunction const& function);

void analyzeLiveRSIVariables(RSIFunction& function);

}