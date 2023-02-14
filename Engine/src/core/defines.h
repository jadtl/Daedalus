#pragma once

namespace ddls {

/** @brief Acknowledges that the given values are unused inside a function */
template<class T>
void ignore(const T &...) {}

#ifdef DDLS_EXPORT
// Exports
#ifdef _MSC_VER
#define DDLS_API __declspec(dllexport)
#else
#define DDLS_API __attribute__((visibility("default")))
#endif
#else
// Imports
#ifdef _MSC_VER
#define DDLS_API __declspec(dllimport)
#else
#define DDLS_API
#endif
#endif

} // namespace ddls