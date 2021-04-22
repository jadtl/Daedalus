#include <string>
#include <vector>

class Platform;

class Engine {
public:
    Engine(const Engine& engine) = delete;
    Engine &operator=(const Engine& engine) = delete;
    virtual ~Engine() {}
    
    struct Settings {
        const char* applicationName;
        const char* engineName;
        
        std::vector<int> windowExtent;
        
        bool validate;
        bool verbose;
    };
    const Settings &settings() const { return settings_; }
    
    enum Key {
        KEY_A,
        KEY_S,
        KEY_D,
        KEY_Q,
        KEY_W,
        KEY_E,
        KEY_SPACE,
    };
    void onKey(Key key);
    
    virtual void onTick() {}
    virtual void onFrame() {}
    
    virtual void update() {}
    
protected:
    Engine(const char* applicationName, const std::vector<std::string>& args) : settings_(), platform_(nullptr) {
        settings_.applicationName = applicationName;
        settings_.engineName = "Engine";
        
        settings_.windowExtent = { 1280, 1024 };
        
        settings_.validate = false;
        settings_.verbose = false;
        
        parseArgs(args);
    }
    
    Settings settings_;
    Platform *platform_;
    
private:
    void parseArgs(const std::vector<std::string>& args) {
        for (auto iterator = args.begin(); iterator != args.end(); ++iterator) {
            if (*iterator == "-validate" || *iterator == "-v")
                settings_.validate = true;
            if (*iterator == "-verbose" || *iterator == "-vv") {
                settings_.validate = true;
                settings_.verbose = true;
            }
        }
    }
};
