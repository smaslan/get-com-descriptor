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
#pragma comment (lib, "Setupapi.lib")

#include <windows.h>
#include <tchar.h>
#include <setupapi.h>
#include <initguid.h>
#include <Usbiodef.h>
#include <devpkey.h>
#include <string>
#include <vector>
#include <regex>
#include <codecvt>
#include <stdio.h>

#include "get_com_desc.h"


// device record
struct TDevice {
	std::wstring DevicePath;
	std::wstring DeviceDesc;
	std::wstring FriendlyName;
	std::wstring BusReportedDeviceDesc;
	std::wstring InstanceId;
	std::wstring Children;
	std::wstring comName;
	int comNum;

	// for sorting function
	bool operator < (const TDevice& rhs) const {
		return comNum < rhs.comNum;
	}
};

// max size of device string descriptors
#define DEV_DESC_SIZE 1024

// enable debug print?
#define DEBUG_PRINT 0

// mode of operation
#define MODE_GET_COM_LIST 0
#define MODE_GET_COM_BY_NAME 1
#define MODE_GET_COM_BY_DESC 2
enum MODE {
	LIST,
	BY_NAME,
	BY_DESC,
	UNKNOWN
};


// convert UTF-8 string to wstring
std::wstring utf8_to_wstring(const std::string& str) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

// convert wstring to UTF-8
std::string wstring_to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}


// list devices of given class GUID
// note: partially inspired by code from here: https://gist.github.com/rakesh-gopal/9ac217aa218e32372eb4
std::vector<TDevice> list_class(const GUID dev_class)
{	
	// list of devices
	std::vector<TDevice> list;

		// list devices of matching class (present devices only)
	HDEVINFO hDevInfo = SetupDiGetClassDevs(&dev_class,NULL,0,DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if(hDevInfo == INVALID_HANDLE_VALUE)
		return(list);
	
	// Prepare to enumerate all device interfaces for the device information
	// set that we retrieved with SetupDiGetClassDevs(..)
	SP_DEVICE_INTERFACE_DATA DevIntfData;
	DevIntfData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	DWORD dwMemberIdx = 0;
	
	while(true)
	{
		// enumerate device interfaces
		SetLastError(0);
		SetupDiEnumDeviceInterfaces(hDevInfo,NULL,&dev_class,dwMemberIdx++,&DevIntfData);
		//if(GetLastError() == ERROR_NO_MORE_ITEMS) /* I guess this is potential endless loop if it returns other error? */
		if(GetLastError())
			break;
			
		// Allocate memory for the DeviceInterfaceDetail struct.
		DWORD dwSize = 0;
		SetupDiGetDeviceInterfaceDetail(hDevInfo,&DevIntfData,NULL,0,&dwSize,NULL);
		PSP_DEVICE_INTERFACE_DETAIL_DATA DevIntfDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,dwSize);
		DevIntfDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// get device details
		SP_DEVINFO_DATA DevData;
		DevData.cbSize = sizeof(DevData);
		if(SetupDiGetDeviceInterfaceDetail(hDevInfo,&DevIntfData,DevIntfDetailData,dwSize,&dwSize,&DevData))
		{				
			#if DEBUG_PRINT
				wprintf(L"%ws:\n",DevIntfDetailData->DevicePath);
			#endif	

			// prepare device record
			TDevice dev;
			dev.DevicePath = DevIntfDetailData->DevicePath;
			dev.comNum = 0;
				
			// get list of available device keys			
			DWORD keyCount = 0;
			SetupDiGetDevicePropertyKeys(hDevInfo,&DevData,NULL,0,&keyCount,0);
			std::vector<DEVPROPKEY> keys(keyCount);
			SetupDiGetDevicePropertyKeys(hDevInfo,&DevData,keys.data(),keys.size(),NULL,0);

			// search through the keys
			for(auto &key: keys)
			{
				DEVPROPTYPE propType;
				wchar_t str[DEV_DESC_SIZE];
				str[0] = L'\0';
					
				if(key == DEVPKEY_Device_FriendlyName)
				{
					// this should be some vendor specific string, for COM ports it should end with "(COMx)" string
					SetupDiGetDevicePropertyW(hDevInfo,&DevData,&key,&propType,(PBYTE)str,DEV_DESC_SIZE,NULL,0);
					dev.FriendlyName = str;
					#if DEBUG_PRINT
						wprintf(L" DEVPKEY_Device_FriendlyName = %ws\n",dev.FriendlyName.c_str());
					#endif

					if(dev_class == GUID_DEVINTERFACE_COMPORT)
					{
						// it is COM port, so try to extract COM port number
						std::wregex pattern(L"\\((COM([0-9])+)\\)");
						std::wsmatch chunks;
						std::regex_search(dev.FriendlyName,chunks,pattern);
						if(chunks.size() == 3)
						{
							dev.comName = chunks[1];
							dev.comNum = std::stoi(chunks[2]);
							#if DEBUG_PRINT
								wprintf(L" COMPORT = %ws\n",dev.comName.c_str());
							#endif
						}
					}
				}
				
				if(key == DEVPKEY_Device_DeviceDesc)
				{
					// this should be manufacturer specific name of device
					SetupDiGetDevicePropertyW(hDevInfo,&DevData,&key,&propType,(PBYTE)str,DEV_DESC_SIZE,NULL,0);
					dev.DeviceDesc = str;
					#if DEBUG_PRINT
						wprintf(L" DEVPKEY_Device_DeviceDesc = %ws\n",dev.DeviceDesc.c_str());
					#endif
				}

				if(key == DEVPKEY_Device_BusReportedDeviceDesc)
				{
					// this should be USB/COM chip name as programmed in its EEPROM - that's the thing we look for
					SetupDiGetDevicePropertyW(hDevInfo,&DevData,&key,&propType,(PBYTE)str,DEV_DESC_SIZE,NULL,0);
					dev.BusReportedDeviceDesc = str;
					#if DEBUG_PRINT
						wprintf(L" DEVPKEY_Device_BusReportedDeviceDesc = %ws\n",dev.BusReportedDeviceDesc.c_str());
					#endif
				}

				if(key == DEVPKEY_Device_InstanceId)
				{
					// this is COM port child device ID that should match parent USB device
					SetupDiGetDevicePropertyW(hDevInfo,&DevData,&key,&propType,(PBYTE)str,DEV_DESC_SIZE,NULL,0);
					dev.InstanceId = str;
					#if DEBUG_PRINT
						wprintf(L" DEVPKEY_Device_InstanceId = %ws\n",dev.InstanceId.c_str());
					#endif
				}

				if(key == DEVPKEY_Device_Children)
				{
					// this USB id should match the children COM port InstanceID
					SetupDiGetDevicePropertyW(hDevInfo,&DevData,&key,&propType,(PBYTE)str,DEV_DESC_SIZE,NULL,0);
					dev.Children = str;
					#if DEBUG_PRINT
						wprintf(L" DEVPKEY_Device_Children = %ws\n",dev.Children.c_str());
					#endif
				}
				
			}					

			// collect device records
			list.push_back(dev);
		}

		HeapFree(GetProcessHeap(),0,DevIntfDetailData);
	}

	// close device list
	SetupDiDestroyDeviceInfoList(hDevInfo);
	
	return(list);
}


// get COM list, or COM by name or COM by descriptor
std::wstring get_com(MODE mode, std::wstring key=L"")
{

	#if DEBUG_PRINT
		wprintf(L"--- USB device list ---\n");
	#endif

	// list USB devices
	auto usb_list = list_class(GUID_DEVINTERFACE_USB_DEVICE);

	#if DEBUG_PRINT
		wprintf(L"\n--- COMPORT device list ---\n");
	#endif

	// list COMPORT devices
	auto com_list = list_class(GUID_DEVINTERFACE_COMPORT);


	// combine COMPORT and USB descriptors
	for(auto& com_dev: com_list)
	{
		// replace not descsiptive names by more sensible ones (very specific rules for particular devices...)
		if(com_dev.BusReportedDeviceDesc.compare(L"Multifunction Device") == 0 && !com_dev.DeviceDesc.empty())
			com_dev.BusReportedDeviceDesc = com_dev.DeviceDesc;

		// try to find parent USB device
		if(com_dev.InstanceId.empty())
			continue;
		for(auto& usb_dev: usb_list)
		{
			if(usb_dev.Children.empty() || usb_dev.Children != com_dev.InstanceId)
				continue;
			// found USB parent

			if(usb_dev.BusReportedDeviceDesc.empty())
				continue;

			// override COMPORT reported name by USB name which should be always valid,
			// unlike COMPORT reported name (at least for FTDI chips)
			com_dev.BusReportedDeviceDesc = usb_dev.BusReportedDeviceDesc;
			break;
		}
	}

	// sort by COMPORT numbers
	std::sort(com_list.begin(),com_list.end());

	std::wstring report;

	// show full list of device?
	if(mode == MODE::LIST)
	{
		for(auto& com_dev: com_list)
			report += com_dev.comName + L"\t" + com_dev.BusReportedDeviceDesc + L"\n";
		return(report);
	}

	// look for device?
	for(auto& com_dev: com_list)
	{
		if(mode == MODE::BY_NAME && com_dev.comName.compare(key) == 0)
			report += com_dev.BusReportedDeviceDesc + L"\n";
		if(mode == MODE::BY_DESC && com_dev.BusReportedDeviceDesc.compare(key) == 0)
			report += com_dev.comName + L"\n";
	}
	
	return(report);
}



// DLL entry: get list of all COM ports to char buffer of max size
__int32 get_com_list(char* buf,__int32 size)
{
	auto report = wstring_to_utf8(get_com(MODE::LIST));
	return(sprintf_s(buf,size,"%s\n",report.c_str()));
}

// DLL entry: get COM name(s) by descriptor string
__int32 get_com_by_desc(char* buf,__int32 size,char* desc)
{
	auto report = wstring_to_utf8(get_com(MODE::BY_DESC,utf8_to_wstring(desc)));
	return(sprintf_s(buf,size,"%s\n",report.c_str()));
}

// DLL entry: get COM port descriptor by COM name string
__int32 get_com_desc(char* buf,__int32 size,char* name)
{
	auto report = wstring_to_utf8(get_com(MODE::BY_NAME,utf8_to_wstring(name)));
	return(sprintf_s(buf,size,"%s\n",report.c_str()));
}

// DLL entry: get lib version string
__int32 get_ver(char* buf, __int32 size)
{
	return(sprintf_s(buf, size, "COM port BusReportedDeviceDesc string extractor, V1.0, " __DATE__ ", (c) Stanislav Maslan"));
}




// console entry point
int main(int argc,char* argv[])
{
	// mode of operation?
	MODE mode = MODE::UNKNOWN;
	if(argc == 2 && std::string(argv[1]).compare("-list") == 0)
		mode = MODE::LIST;
	else if(argc == 3 && std::string(argv[1]).compare("-name") == 0)
		mode = MODE::BY_NAME;
	else if(argc == 3 && std::string(argv[1]).compare("-desc") == 0)
		mode = MODE::BY_DESC;
	
	if(mode == MODE::UNKNOWN)
	{
		// show help
		wprintf(L"COM port BusReportedDeviceDesc string extractor.\n");
		wprintf(L"(c) Stanislav Maslan, V1.0, " __DATE__ "\n");
		wprintf(L"\nusage examples:\n");
		wprintf(L"get_com_descriptor.exe\n");
		wprintf(L" - shows help\n\n");
		wprintf(L"get_com_descriptor.exe -list\n");
		wprintf(L" - show list of all COM ports and their BusReportedDeviceDesc strings\n\n");
		wprintf(L"get_com_descriptor.exe -name COM8\n");
		wprintf(L" - show BusReportedDeviceDesc string for given COM name\n\n");
		wprintf(L"get_com_descriptor.exe -desc \"FTDI usb bridge\"\n");
		wprintf(L" - show COM names matching given BusReportedDeviceDesc string\n");
		return(0);
	}

	// port search key
	std::wstring key;
	if(mode != MODE::LIST)
		key = utf8_to_wstring(argv[2]);		

	// do stuff
	auto report = get_com(mode, key);

	// print result
	wprintf(L"%ws\n", report.c_str());

	return(0);
}