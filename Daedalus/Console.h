#include <string>

class Console {
public:
    Console();
    
    enum LogCategory {
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR,
    };
    virtual void log(LogCategory category, std::string message) const;
private:
    
};
