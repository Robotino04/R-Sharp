#pragma once

#include <iostream>
#include <stdexcept>
#include <list>
#include <algorithm>

template<typename ...Args>
inline void Fatal(Args&& ...args);

namespace Internals{
    const std::string ansiColorStart = "\033[";
    const std::string ansiColorEnd = "m";

    const std::string colorReset =         ansiColorStart + "0"  + ansiColorEnd;

    const std::string colorBlack =         ansiColorStart + "30" + ansiColorEnd;
    const std::string colorRed =           ansiColorStart + "31" + ansiColorEnd;
    const std::string colorGreen =         ansiColorStart + "32" + ansiColorEnd;
    const std::string colorYellow =        ansiColorStart + "33" + ansiColorEnd;
    const std::string colorBlue =          ansiColorStart + "34" + ansiColorEnd;
    const std::string colorMagenta =       ansiColorStart + "35" + ansiColorEnd;
    const std::string colorCyan =          ansiColorStart + "36" + ansiColorEnd;
    const std::string colorWhite =         ansiColorStart + "37" + ansiColorEnd;
    const std::string colorBrightBlack =   ansiColorStart + "90" + ansiColorEnd;
    const std::string colorBrightRed =     ansiColorStart + "91" + ansiColorEnd;
    const std::string colorBrightGreen =   ansiColorStart + "92" + ansiColorEnd;
    const std::string colorBrightYellow =  ansiColorStart + "93" + ansiColorEnd;
    const std::string colorBrightBlue =    ansiColorStart + "94" + ansiColorEnd;
    const std::string colorBrightMagenta = ansiColorStart + "95" + ansiColorEnd;
    const std::string colorBrightCyan =    ansiColorStart + "96" + ansiColorEnd;
    const std::string colorBrightWhite =   ansiColorStart + "97" + ansiColorEnd;

    template <typename T>
    void printToStream(std::ostream& stream, T&& val) {
        stream << val;
    }
    
    template <typename T, typename ...Args>
    void printToStream(std::ostream& stream, T &&val, Args&& ...args) {
        printToStream(stream, val);
        printToStream(stream, args...);
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
    Internals::printToStream(std::cerr, Internals::colorRed, "[ERROR]", Internals::getContext(), ": ", Internals::colorReset, args..., '\n');
    exit(1);
}
template<typename ...Args>
inline void Error(Args&& ...args){
    Internals::registerError();
    Internals::printToStream(std::cerr, Internals::colorRed, "[ERROR]", Internals::getContext(), ": ", Internals::colorReset, args..., '\n');
}
template<typename ...Args>
inline void Warning(Args&& ...args){
    Internals::printToStream(std::clog, Internals::colorYellow, "[WARNING]", Internals::getContext(), ": ", Internals::colorReset, args..., '\n');
}
template<typename ...Args>
inline void Log(Args&& ...args){
    Internals::printToStream(std::clog, Internals::colorBrightBlue, "[LOG]", Internals::getContext(), ": ", Internals::colorReset, args..., '\n');
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