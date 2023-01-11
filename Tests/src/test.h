#pragma once

#include <defines.h>

#define ASSERT_THROWS(expression, ExceptionType)\
   bool _exceptionThrown = false;\
   try {\
      expression;\
   } catch ( const ExceptionType & ) {\
      _exceptionThrown = true;\
   }\
   ASSERT(_exceptionThrown)\

#define TEST_SUCCESS\
    std::cout << "Test passed!" << '\n';\
