#pragma once

#include "R-Sharp/backend/RSI_FWD.hpp"

std::string rsiToAarch64(RSI::Function const& function);
std::string rsiToNasm(RSI::Function const& function);
