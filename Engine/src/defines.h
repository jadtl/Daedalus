#pragma once

namespace ddls {

/** @brief Aknowledges that the given values are unused inside a function */
template<class T> void ignore(const T&...) {}

} // namespace ddls