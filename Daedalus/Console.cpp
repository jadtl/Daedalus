#include "Console.h"

#include <iostream>

Console::Console() {}

void Console::log(std::string section, LogCategory category, std::string message) const {
    std::ostream &stream = (category == LOG_ERROR) ? std::cerr : std::cout;
    std::string toString;
    switch(category) {
        case LOG_VERBOSE:
            toString = "VERBOSE";
            break;
        case LOG_INFO:
            toString = "INFO";
            break;
        case LOG_WARNING:
            toString = "WARNING";
            break;
        case LOG_ERROR:
            toString = "ERROR";
            break;
        case LOG_FATAL:
            toString = "FATAL";
    }
    if (category == LOG_FATAL)
        throw new std::runtime_error("[" + toString + "]\n" + message);
    else
        stream << "[" << toString << ": " << section << "]\n" << message << "\n";
}
