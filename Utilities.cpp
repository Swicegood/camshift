#include "Utilities.h"
#include <sstream>
#include <Windows.h>


void DebugOutputW(const std::wstring& message) {
    OutputDebugStringW((message + L"\n").c_str());
}