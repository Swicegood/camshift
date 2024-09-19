#pragma once
#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <sstream>

template<typename T>
std::wstring toWString(const T& value) {
    std::wostringstream woss;
    woss << value;
    return woss.str();
}

void DebugOutputW(const std::wstring& message);

#endif // UTILITIES_H