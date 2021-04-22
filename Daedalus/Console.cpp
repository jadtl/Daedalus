#include "Console.h"

#include <iostream>

Console::Console() {}

void Console::log(LogCategory category, std::string message) const {
    std::ostream &stream = (category == LOG_ERROR) ? std::cerr : std::cout;
    const char* toString;
    switch(category) {
        case LOG_DEBUG:
            toString = "DEBUG";
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
    }
    stream << "[" << toString << "] " << message << "\n";
}
