#pragma once

#include <boost/filesystem.hpp>

#include <string>

class Explorer {
public:
    Explorer &operator=(const Explorer &explorer) = delete;
    virtual ~Explorer() {}
    
    std::string shader(std::string fileName) const;
    std::string asset(std::string fileName) const;
    
protected:
    boost::filesystem::path programRoot;

private:
    virtual boost::filesystem::path executable() = 0;
    virtual std::string currentWorkingDirectory() = 0;
};
