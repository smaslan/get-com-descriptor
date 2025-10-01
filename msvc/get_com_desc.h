//-----------------------------------------------------------------------------
// Simple tool to obtain user programmed names of USB-COM port chips.
// The names are usually stored in internal or external EEPROM.
// Problem is that different chips stores the user string in different 
// device descriptors, so it's quite a mess to get universal approach.
// e.g. Microchip MCP2200 or Silicon Labs CP2102 stores the user string
// in COM port descriptor BusReportedDeviceDesc. But FTDI chips seems to
// store the user string in matching USB device parent's BusReportedDeviceDesc.
// This tool is trying to automate this process and return the user strings.
// It can either list all COM ports with their descriptors or it can find
// COM port by descriptor or it can find descriptor by COM port name.
// 
// Example: get_com_descriptor.exe -list
//   will return (one row per device, tab separated):
// COM2    AX99100 PCIe to High Speed Serial Port
// COM3    MK3-USB Interface
// COM4    AX99100 PCIe to High Speed Serial Port
// COM6    CP2102N(Toslink Bridge)
// COM7    TFA Meteo Logger(CP2102N USB to UART)
// COM8    CP2102N(DC bus TRX)
//
// Example: get_com_descriptor.exe -name COM6
//   will return:
// CP2102N(Toslink Bridge)
// 
// Example: get_com_descriptor.exe -desc "CP2102N(Toslink Bridge)"
//   will return:
// COM6
//
// Example: get_com_descriptor.exe -desc "AX99100 PCIe to High Speed Serial Port"
//   will return (one row per COM port if more than one matches):
// COM2
// COM4
// 
// Notes: The code was inspired by some online code snippets.
// Also, there might be a problem with wstring when using localized names.
// I'm not sure how to solve this properly as I need to use this e.g.
// in LabVIEW which cannot handle wstrings or utf8.
// 
// (c) Stanislav Maslan, s.maslan@seznam.cz, v1.0 2025-04-16
// https://github.com/smaslan/get-com-descriptor
// This project is distributed under MIT license.
//-----------------------------------------------------------------------------

#pragma once

#ifdef _LVPDLLEXPORT
#define DllExport __declspec(dllexport) 
#else
#define DllExport __declspec(dllimport) 
#endif


// DLL entry points
DllExport __int32 get_com_list(char* buf,__int32 size,__int32 to_ascii);
DllExport __int32 get_com_by_desc(char* buf,__int32 size,char* desc,__int32 to_ascii);
DllExport __int32 get_com_desc(char* buf,__int32 size,char* name,__int32 to_ascii);
DllExport __int32 get_ver(char* buf,__int32 size);

