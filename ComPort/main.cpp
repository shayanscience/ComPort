/* 
	Project: PortPilot
	Author:			Shayan Sheikhrezaei
	Email:			Shayan@orcatechnologies.com
	Start Date:		01/29/2026
	First Release:	01/30/2026
	
	
	
	Your serial configuration is correct.
⭐ Your write logic is correct.
⭐ Your read logic is correct in principle.
*/

/*		Data			Release Version				Notes
* *************************************************************
*	01/30/2026				1.0.0				Inital Release
*
*
*/

#include <iostream>
#include <Windows.h>
#include <string>
#include <fileapi.h>
#include <WinBase.h>

int main() {
	wchar_t devices[65535];		// wchar_t (wide character type) 16-bit, has capacity to stor 65535 devices.
	char buffer[32] = { 0 };	// buffer is used to receive bytes from the other device - can receive up to 32bytes of information.
	DCB dcb;					// dcb(data control block) is a API structure for configuration of the serial communication. It contains info such as: baudRate, Parity, and etc.
	COMMTIMEOUTS timeOut;		// API structure to configure the timeout parameters.
	COMSTAT comStat = { 0 };	// COMSTAT is an API structure that is used to check how many bytes are available on the slave device.
	DWORD errors = 0;			// Used in ClearComStat() function 
	DWORD bytesRead = 0;		// Used in ReadFile() so that it shows how many bytes have been read by the system.
	

	// Function below will look into all devices connected to the system and return their size (number).
	// This fills the 'devices' variable with port names.
	DWORD size = QueryDosDevice(NULL, devices, sizeof(devices));


	// If zero, we can output what error occured, useful for debugging.
	if (size == 0) {
		std::cout << "Error: " << GetLastError() << std::endl;
		return 1;
	}

	//Stores the address of devices into ptr pointer.
	wchar_t* ptr = devices;

	while (*ptr) { // loops through all the available devices until it hits '\0'
		std::wstring name(ptr);
		if (name.find(L"COM") == 0) {
			std::wcout << L"Found port: " << name << std::endl;
		}
		ptr += wcslen(ptr) + 1;
	}


	HANDLE hfile = CreateFileA(
		"COM7",
		(GENERIC_READ | GENERIC_WRITE),
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hfile == INVALID_HANDLE_VALUE) {
		std::cout << " Could not open the COM port";
	}
	else {
		std::cout << "COM port is opened" << std::endl;
		bool device_control_Getstate = GetCommState(hfile, &dcb);
		if(device_control_Getstate == 0){
			std::cout << "Error: " << GetLastError() << std::endl;
		}
		else {
			dcb.BaudRate = 9600;
			dcb.ByteSize = 8;
			dcb.StopBits = 1;
			dcb.Parity = 0;

		}

		bool device_control_SetState = SetCommState(hfile, &dcb);
		if (device_control_SetState == 0) {
			std::cout << "Error: " << GetLastError() << std::endl;
		}
		else {
			std::cout << "DCB Length: " << dcb.DCBlength << std::endl;
			std::cout << "BaudRate: " << dcb.BaudRate << std::endl;
			std::cout << "Parity : " << dcb.Parity << std::endl;

		}

		bool comm_timeOut_Getstate = GetCommTimeouts(hfile, &timeOut);
		if (comm_timeOut_Getstate ==0) {
			std::cout << "Error: " << GetLastError() << std::endl;
		}
		else {
			timeOut.ReadIntervalTimeout = 50;
			timeOut.ReadTotalTimeoutMultiplier = 0;
			timeOut.ReadTotalTimeoutConstant = 50;
		}

		bool comm_timeOut_Setstate = SetCommTimeouts(hfile, &timeOut);
		if (comm_timeOut_Getstate == 0) {
			std::cout << "Error: " << GetLastError() << std::endl;
		}
		else {
			std::cout << " " << std::endl;
			std::cout << "ReadIntervalTimeout: " << timeOut.ReadIntervalTimeout << std::endl;
			std::cout << "ReadTotalTimeoutMultiplier: " << timeOut.ReadTotalTimeoutMultiplier << std::endl;
			std::cout << "ReadTotalTimeoutConstant : " << timeOut.ReadTotalTimeoutConstant << std::endl;
			std::cout << "Time Out Has successfully been set!" << std::endl;
		}


		bool write_status = WriteFile(
			hfile,
			"help\n",
			5,
			NULL,
			NULL
		);

		if (write_status == 0) {
			std::cout << "Error: " << GetLastError() << std::endl;
		}
		else {
			std::cout << "Message successfully written!" <<  std::endl;
		}

		bool com_status = ClearCommError(
			hfile,
			&errors,
			&comStat
		);


		while (!comStat.cbInQue) {
			bool com_status = ClearCommError(
				hfile,
				&errors,
				&comStat
			);
		};

		if (com_status == 0) {
			std::cout << "Error: " << GetLastError() << std::endl;
		}
		else {
			std::cout << comStat.cbInQue <<" Bytes waiting to be read!" << std::endl;
		}

		bool read_status = ReadFile(
			hfile,
			buffer,
			sizeof(buffer),
			&bytesRead,
			NULL
		);

		buffer[bytesRead] = '\0';
		if (read_status == 0) {
			std::cout << "Error: " << GetLastError() << std::endl;
		}
		else {
			std::cout << "Message successfully has been received!" << std::endl;
			std::cout << "\nReceived Message: " << buffer << std::endl;
		}

		CloseHandle(hfile);
		std::cout << "Handle is closed" << std::endl;
	}

	return 0;
}
