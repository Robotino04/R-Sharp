#pragma once

#include <iostream>
#include <stdexcept>
#include <list>
#include <algorithm>

#include "ANSI/ANSI.hpp"

template<typename ...Args>
inline void Fatal(Args&& ...args);

namespace Internals{
    template <typename ...Args>
    void printToStream(std::ostream& stream, Args&& ...args) {
        (stream << ... << args);
    }

    inline std::list<const char*> contextPath = {};

    inline const std::string getContext(){
        std::string context = "";
        for (auto node : contextPath){
            context = context + '[' + node + ']';
        }
        return context;
    }

    class LoggingContextImpl{
        public:
            LoggingContextImpl(const char* contextName): name(contextName){
                Internals::contextPath.push_back(contextName);
            }
            ~LoggingContextImpl(){
                auto x = std::find(Internals::contextPath.rbegin(), Internals::contextPath.rend(), name);
                if (x != Internals::contextPath.rend())
                    Internals::contextPath.erase(std::next(x).base());
            }

        private:
            const char* name;
    };

    inline int errorCount = 0;
    inline int errorLimit = -1;

    inline void registerError(){
        errorCount++;
        if (errorLimit > 0 && errorCount > errorLimit){
            Fatal("Too many errors");
        }
    }
}


template<typename ...Args>
inline void Fatal(Args&& ...args){
    Internals::printToStream(std::cout, ANSI::set4BitColor(ANSI::Red), "[ERROR]", Internals::getContext(), ": ", ANSI::reset(), args..., '\n');
    exit(1);
}
template<typename ...Args>
inline void Error(Args&& ...args){
    Internals::registerError();
    Internals::printToStream(std::cout, ANSI::set4BitColor(ANSI::Red), "[ERROR]", Internals::getContext(), ": ", ANSI::reset(), args..., '\n');
}
template<typename ...Args>
inline void Warning(Args&& ...args){
    Internals::printToStream(std::cout, ANSI::set4BitColor(ANSI::Yellow), "[WARNING]", Internals::getContext(), ": ", ANSI::reset(), args..., '\n');
}
template<typename ...Args>
inline void Log(Args&& ...args){
    Internals::printToStream(std::cout, ANSI::set4BitColor(ANSI::BrightBlue), "[LOG]", Internals::getContext(), ": ", ANSI::reset(), args..., '\n');
}
template<typename ...Args>
inline void Print(Args&& ...args){
    Internals::printToStream(std::cout, args..., '\n');
}

inline void setErrorLimit(int limit){
    Internals::errorLimit = limit;
}
inline void resetErrorCount(){
    Internals::errorCount = 0;
}
inline int getErrorCount(){
    return Internals::errorCount;
}

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b

#define LoggingContext(NAME) const Internals::LoggingContextImpl CONCAT(___loggingContext, __COUNTER__)(NAME)