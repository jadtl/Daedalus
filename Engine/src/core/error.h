#pragma once

#include <iostream>
#include <string>

#include "types.h"

namespace ddls {

using Boolean = u8;

class CustomMessageException : public std::exception
{
private:
    std::string _msg;

public:
    CustomMessageException(std::string msg) : _msg(msg) {}

    std::string what() { return _msg; }
};

class DDLS_API OutOfMemoryException : public CustomMessageException
{
public:
    OutOfMemoryException(std::string msg) : CustomMessageException(msg) {}
};

class DDLS_API OutOfBoundsException : public CustomMessageException
{
public:
    OutOfBoundsException(std::string msg) : CustomMessageException(msg) {}
};

} // namespace ddls