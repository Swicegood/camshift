#pragma once

#include <Windows.h>
#include <dshow.h>
#include <map>
#include <string>

#pragma comment(lib, "strmiids")

// Struct to hold device information
struct Device {
    int id;                 // Device ID for OpenCV VideoCapture
    std::string devicePath; // Device path (not used in this context)
    std::string deviceName; // Device name for display
};

class DeviceEnumerator {
public:
    DeviceEnumerator() = default;

    // Methods to get device maps
    std::map<int, Device> getDevicesMap(const GUID deviceClass);
    std::map<int, Device> getVideoDevicesMap();
    std::map<int, Device> getAudioDevicesMap();

private:
    // Helper methods for string conversion
    std::string ConvertBSTRToMBS(BSTR bstr);
    std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen);
};
