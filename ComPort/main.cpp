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
*	01/05/2026				1.0.1				Added more comments for better understanding.
*
*/

#include <iostream>
#include <Windows.h>
#include <string>
#include <fileapi.h>
#include <WinBase.h>


/// <summary>
/// This function receives dcb struct.
/// This function has default serial port settings.
///		BuadRate = 9600
///		ByteSize = 8
/// 	Parity = NOPARITY
///		StopBits = ONESTOPBIT
/// </summary>
/// <param name="dcb"></param>
void serialPortSetup(
	// below we attempt to configure com por configuration.
	DCB* dcb,
	DWORD baudRate,
	BYTE byteSize,
	BYTE parity,
	BYTE stopBits
) {

	dcb->BaudRate = baudRate;
	dcb->ByteSize = byteSize;
	dcb->Parity = parity;
	dcb->StopBits = stopBits;

}

// Serial port setup wrapper. Following method enables the user to change the setup default values
//	as needed. The user can use following function as follow:
//		serialPortSetup(DCB structure)		or		serialPortSetup(DCB struct, buadrate , byte size, parity, stopbit);
void serialPortSetup(DCB* dcb) {		/// Function added
	serialPortSetup(
		dcb,
		CBR_9600,
		8,
		NOPARITY,
		ONESTOPBIT
	);
}


/// <summary>
/// write_message(HANDLE hfile, std::string& msg, DWORD* bytesWritten) 
/// hfile -> File handler for the created file. Appropriate COMPORT is assigned in this handler.
/// msg	  -> Takes user input.
/// bytesWritten	-> Populates the number of bytes written in 'bytesWritten'.
/// </summary>
/// <param name="hfile"></param>
/// <param name="msg"></param>
/// <param name="bytesWritten"></param>
/// <returns></returns>
bool write_message(HANDLE hfile, std::string& msg, DWORD* bytesWritten) {
	return WriteFile(
		hfile,
		msg.c_str(),	// Retrieves the string.
		msg.length(),	// Gets length of the string.
		bytesWritten,
		NULL
	);
}

int main() {
	wchar_t devices[65535];		// wchar_t (wide character type) 16-bit, has capacity to stor 65535 devices.
	char read_buffer[32] = { 0 };	// buffer is used to receive bytes from the other device - can receive up to 32bytes of information.
	COMMTIMEOUTS timeOut;		// API structure to configure the timeout parameters.
	DCB dcb;					// dcb(data control block) is a API structure for configuration of the serial communication. It contains info such as: baudRate, Parity, and etc.
	COMSTAT comStat = { 0 };	// COMSTAT is an API structure that is used to check how many bytes are available on the slave device.
	DWORD errors = 0;			// Used in ClearComStat() function 
	DWORD bytesRead = 0;		// Used in ReadFile() so that it shows how many bytes have been read by the system.
	DWORD bytesWritten;			// This variabl stores number of bytes written to the target by the host.
	DWORD start_time = GetTickCount64();	// Getting current tick.
	DWORD timeout_limit = 2000;				// Conditional variable
	bool time_out = false;					// Time out flag.
	std::string write_buffer;				// Stores user input.

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

	// Create a file handle with the given field such as: only read/write or both, and etc. for a specific port
	//	and for this case is for 'COM7'.
	HANDLE hfile = CreateFileA(
		"COM7",
		(GENERIC_READ | GENERIC_WRITE),
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	// Check if file creation succeeded
	if (hfile == INVALID_HANDLE_VALUE) {
		std::cout << " Could not open the COM port";
	}// if successful get default communication states (baudrate, size, parity, etc.).
	else {
		std::cout << "COM port is opened" << std::endl;
		bool device_control_Getstate = GetCommState(hfile, &dcb);

		if (device_control_Getstate == 0) { // if getting the port configuration failed, output the error value.
			std::cout << "Error: " << GetLastError() << std::endl;
			return 1;
		}


		serialPortSetup(&dcb, 9600, 8, NOPARITY, ONESTOPBIT);
		//serialPortSetup(&dcb);
		/// Commented function above is used for testing 

		// SetCommState will set above values to the port configuration.
		bool device_control_SetState = SetCommState(hfile, &dcb);
		if (device_control_SetState == 0) {
			std::cout << "Error: " << GetLastError() << std::endl;
			return 1;
		}

		// Below we attempt to read a few state parameters to confirm above com port configuration.
		std::cout << "DCB Length: " << dcb.DCBlength << std::endl;
		std::cout << "BaudRate: " << dcb.BaudRate << std::endl;
		std::cout << "Parity : " << dcb.Parity << std::endl;

		// Reading communication timeouts.
		bool comm_timeOut_Getstate = GetCommTimeouts(hfile, &timeOut);
		if (comm_timeOut_Getstate == 0) {
			std::cout << "Error: " << GetLastError() << std::endl;
			return 1;
		}

		// we set read variables. I picked 50ms for interval timeout and timeout constant.
		timeOut.ReadIntervalTimeout = 50;
		timeOut.ReadTotalTimeoutMultiplier = 0;
		timeOut.ReadTotalTimeoutConstant = 50;

		// Values above are written into SetCommTimeouts configs.
		bool comm_timeOut_Setstate = SetCommTimeouts(hfile, &timeOut);
		if (comm_timeOut_Getstate == 0) {
			std::cout << "Error: " << GetLastError() << std::endl;
			return 1;
		}

		// Reading values to confirm they are set properly.
		std::cout << " " << std::endl;
		std::cout << "ReadIntervalTimeout: " << timeOut.ReadIntervalTimeout << std::endl;
		std::cout << "ReadTotalTimeoutMultiplier: " << timeOut.ReadTotalTimeoutMultiplier << std::endl;
		std::cout << "ReadTotalTimeoutConstant : " << timeOut.ReadTotalTimeoutConstant << std::endl;
		std::cout << "Time Out Has successfully been set!" << std::endl;


		// While loop below, repeats the processes. It waits for the user input.
		//	Depending on how many options available, it will respond accordingly until user envoke "q" which
		//	stands for quit, then the process will end and the system closes the port.
		while(true){
			std::cin >> write_buffer;
			// We attempt to write the message 'help'. It is worthwhile to mention that our target device is set to listent
			// to 'help' once it receives it, it will respond appropriately.
			bool write_status = write_message(hfile, write_buffer, &bytesWritten);
			// Below check the return value from 'WriteFile' whether or not the write was successful.
			if (write_status == 0) {
				std::cout << "Error: " << GetLastError() << std::endl;
				return 1;
			}
			else {
				std::cout << "Message successfully written!" << std::endl;
			}

			bool comm_error_status;

			// 'ClearCommError' it is a windows API that reports Com-port errors and updates COMSTAT structure.
			/*	Note on COMSTAT Structure:
			*							This structure contains a stats reporting number of bytes waiting. Without calling
			*							this function the 'cblnQue' won't get updated.
			*	Below we wait for as long as 'cbInQue' remains zero.
			*	It indicates that there are bytes waiting to be read if it is not zero.
			*/
			do {
				comm_error_status = ClearCommError(
					hfile,
					&errors,
					&comStat
				);
				if (GetTickCount64() - start_time > timeout_limit) {
					time_out = true;
					break;
				}
				//std::cout << "comm error: " << errors << std::endl;
			} while (!comStat.cbInQue);

			if (time_out) {
				std::cout << "Timeout: No response from target!" << std::endl;
			}

			// If we get zero from the function 'ClearCommError()' it indicates there has been an issue.
			if (comm_error_status == 0) {
				std::cout << "Error: " << GetLastError() << std::endl;
			}
			else {// If we succeed we can check how many bytes are there to be read.
				std::cout << comStat.cbInQue << " Bytes waiting to be read!" << std::endl;
			}


			// If there exist any bytes waiting to be read. We begin to read the information, and store it in 'buffer'.
			bool read_status = ReadFile(
				hfile,
				read_buffer,
				sizeof(read_buffer),
				&bytesRead,
				NULL
			);

			// We need to use null terminator to display the end of the message.
			read_buffer[bytesRead] = '\0';

			// Checking the 'read_status' for debugging purposes.
			if (read_status == 0) {
				std::cout << "Error: " << GetLastError() << std::endl;
			}
			else { // Outputting message showing that the read process was successful, and the message will be shown respectively.
				std::cout << "Message successfully has been received!" << std::endl;
				std::cout << "\nReceived Message: " << read_buffer << std::endl;
			}

			// We close the communication line by envoking 'CloseHandle(handle file)'.
			if (write_buffer == "q") {
				CloseHandle(hfile);
				std::cout << "Handle is closed" << std::endl;
				return 1;
			}
		}
	}

	return 0;
}
