#include <string>

class Console {
public:
    Console();
    
    enum LogCategory {
        LOG_VERBOSE,
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR,
        LOG_FATAL,
    };
    virtual void log(std::string section, LogCategory category, std::string message) const;
private:
    
};
