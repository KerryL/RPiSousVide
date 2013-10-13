// File:  uartOneWireInterface.cpp
// Date:  10/8/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  C++ wrapper around UART interface for communicating with 1-wire
//        peripherals.  See
//        http://www.maximintegrated.com/app-notes/index.mvp/id/214 for more
//        information.

// Standard C++ headers
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <cassert>

// *nix standard headers
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

// Local headers
#include "uartOneWireInterface.h"

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		Constant Definitions/Static Member Initialization
//
// Description:		Constand definitions for UARTOneWireInterface class.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
const speed_t UARTOneWireInterface::normalBaud = B115200;
const speed_t UARTOneWireInterface::resetBaud = B9600;
const unsigned char UARTOneWireInterface::searchROMCommand = 0xF0;
//const unsigned char UARTOneWireInterface::readROMCommand = 0x33;// search ROM is preferred
const unsigned char UARTOneWireInterface::matchROMCommand = 0x55;
const unsigned char UARTOneWireInterface::skipROMCommand = 0xCC;
const unsigned char UARTOneWireInterface::alarmSearchCommand = 0xEC;

std::ostream& UARTOneWireInterface::outStream = std::cout;
std::string UARTOneWireInterface::ttyFile = "/dev/ttyS0";
unsigned int UARTOneWireInterface::deviceCount = 0;
int UARTOneWireInterface::serialFile = -1;

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		UARTOneWireInterface
//
// Description:		Constructor for UARTOneWireInterface class.
//
// Input Arguments:
//		rom			= cosnt std::string& indicating the 64-bit ROM code used
//					  to identify slave devices
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
UARTOneWireInterface::UARTOneWireInterface(const std::string &rom) : rom(rom)
{
	if (!CRCIsOK(rom))
	{
		outStream << "Error:  Specified ROM is invalid (CRC)" << std::endl;
		return;
	}

	OpenSerialFile();
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		~UARTOneWireInterface
//
// Description:		Destructor for UARTOneWireInterface class.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
UARTOneWireInterface::~UARTOneWireInterface()
{
	CloseSerialFile();
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		ResetAndPresenceDetect (static)
//
// Description:		Issues reset command and returns a bool indicating whether
//					or not any slave devices responsed by indicating there presence.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for slaves present, false for error or no slaves present
//
//==========================================================================
bool UARTOneWireInterface::ResetAndPresenceDetect(void)
{
	if (!OpenSerialFile())
		return false;

	if (!SetBaud(resetBaud))
		return false;

	if (!Write(searchROMCommand))
		return false;

	unsigned char response;
	if (read(serialFile, &response, sizeof(response)) == -1)
	{
		outStream << "Failed to read from serial port during presence detect:  "
			<< std::strerror(errno) << std::endl;
		return false;
	}

	if (!SetBaud(normalBaud))
		return false;

	if (!CloseSerialFile())
		return false;

	return response != searchROMCommand;
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		Select
//
// Description:		Resets and selects the slave device based on the ROM code.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for succss, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::Select(void) const
{
	return ResetAndPresenceDetect() && Write(matchROMCommand) && Write(rom);
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		GetPresentROMs (static)
//
// Description:		Returns a vector of ROMs of attached devices.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		roms	= std::vector<std::string>&
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::SearchROMs(std::vector<std::string>& roms)
{
	return FindAllDevicesWithCommand(searchROMCommand, roms);
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		AlarmSearch (static)
//
// Description:		Returns a vector of ROMs that are currently alarmed.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		roms	= std::vector<std::string>&
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::AlarmSearch(std::vector<std::string>& roms)
{
	return FindAllDevicesWithCommand(alarmSearchCommand, roms);
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		FindAllDevicesWithCommand (static)
//
// Description:		Returns a vector of ROMs respond to the specified command.
//
// Input Arguments:
//		command	= const unsigned char&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::FindAllDevicesWithCommand(
	const unsigned char &command, std::vector<std::string>& roms)
{
	if (!OpenSerialFile())
		return false;

	roms.clear();

	unsigned char c1, c2, nextValue;
	const unsigned int romLength(64);
	unsigned int lastDiscrepancy(0), lastZeroDiscrepancy, bitNumber;
	bool done(false);

	// We keep a local list because we need to convert to hex before passing it back
	std::vector<std::string> localROMs;
	std::string romBuffer(romLength, '0');

	// Loop until we've found all connected devices
	while (!done)
	{
		if (!ResetAndPresenceDetect() ||
			!Write(command))
		{
			CloseSerialFile();
			return false;
		}

		bitNumber = 0;
		lastZeroDiscrepancy = 0;

		while (bitNumber < romLength)
		{
			if (!Read(c1) || !Read(c2))
			{
				CloseSerialFile();
				return false;
			}
		
			// If both reads are high, assume no devices are connected
			if (c1 == 1 && c2 == 1)
			{
				// ROM vector should be empty - if it's not, tell the user
				if (roms.size() > 0 || bitNumber > 0)
				{
					outStream << "Error:  Expected response from connected device, but no response was received" << std::endl;
					CloseSerialFile();
					return false;
				}

				break;
			}
			// If c1 and c2 are different, all slaves have value c1 at current position
			else if (c1 != c2)
				nextValue = c1;
			// If c1 and c2 are both 0, multiple slaves exist with both a 0 and a 1 at this position
			else
			{
				if (bitNumber + 1 == lastDiscrepancy)
					nextValue = 1;
				else if (bitNumber + 1 > lastDiscrepancy)
					nextValue = 0;
				else
					nextValue = romBuffer[bitNumber] - 48;// -48 to convert from "0" or 1 to 0 or 1

				if (nextValue == 0)
					lastZeroDiscrepancy = bitNumber + 1;
			}

			// Write the current bit to 1-wire to tell matching devices to stay active
			if (!Write(nextValue))
				return false;
			romBuffer[bitNumber] = nextValue + 48;// +48 to convert from 0 or 1 to "0" or "1"
			bitNumber++;
		}

		// We won't hit this unless there weren't any responses
		if (bitNumber == 0)
			break;

		localROMs.push_back(romBuffer);
		lastDiscrepancy = lastZeroDiscrepancy;
		if (lastDiscrepancy == 0)
			done = true;
	}

	if (!CloseSerialFile())
		return false;

	unsigned int i, j;
	uint64_t value;
	std::stringstream ss;
	for (i = 0; i < localROMs.size(); i++)
	{
		ss.str("");
		value = 0;

		for (j = 0; j < localROMs[i].length(); j++)
		{
			value <<= 1;
			if (localROMs[i][j] == '1')
				value |= 0x01;
		}

		ss << std::hex << value;
		roms.push_back(ss.str());

		if (!CRCIsOK(roms[i]))
		{
			outStream << "CRC check failed for ROM " << roms[i] << std::endl;
			return false;
		}
	}

	return true;
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		Write
//
// Description:		Writes one character to the serial port.
//
// Input Arguments:
//		c	= const unsigned char&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::Write(const unsigned char &c)
{
	assert(serialFile);

	if (write(serialFile, &c, sizeof(c)) == -1)
	{
		outStream << "Failed to write to serial port:  "
			<< std::strerror(errno) << std::endl;
		return false;
	}

	if (tcdrain(serialFile) == -1)
	{
		outStream << "Failed to flush serial port following write:  "
			<< std::strerror(errno) << std::endl;
		return false;
	}

	return true;
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		Write
//
// Description:		Writes string to the serial port.
//
// Input Arguments:
//		c	= const std::string&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::Write(const std::string &c)
{
	unsigned int i;
	for (i = 0; i < c.length(); i++)
	{
		if (!Write(c[i]))
			return false;
	}

	return true;
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		Read
//
// Description:		Reads one character (actually, just one bit) from the serial port.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		c	= unsigned char&
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::Read(unsigned char &c)
{
	assert(serialFile);

	// Read time slots start with a write - we write 0xFF which signals (via the start bit)
	// that the slave device should start writing
	unsigned char readSignal(0xFF);
	if (write(serialFile, &readSignal, sizeof(readSignal)) == -1)
	{
		outStream << "Failed to initiate read time slot:  "
			<< std::strerror(errno) << std::endl;
		return false;
	}

	if (read(serialFile, &c, sizeof(c)) == -1)
	{
		outStream << "Failed to read from serial port:  "
			<< std::strerror(errno) << std::endl;
		return false;
	}

	// The response will vary based on device timing, but the entire slot will either
	// equate to a zero or a one.  Check the LSB to decide which it is.
	c &= 0x1;

	return true;
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		OpenSerialFile (static)
//
// Description:		Opens the serial file (if it's not already open) and
//					increments the device counter.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::OpenSerialFile(void)
{
	deviceCount++;
	if (serialFile)
		return true;

	serialFile = open(ttyFile.c_str(), O_RDWR, O_NONBLOCK);
	if (serialFile == -1)
	{
		deviceCount--;
		outStream << "Failed to open '" << ttyFile << "':  "
			<< std::strerror(errno) << std::endl;
		return false;
	}

	// Configure the file with appropriate options for 1-wire interface
	struct termios options;
	if (tcgetattr(serialFile, &options) == -1)
	{
		outStream << "Failed to get serial port options:  "
			<< std::strerror(errno) << std::endl;
		CloseSerialFile();
		return false;
	}

	cfmakeraw(&options);// No parity bit, 8-bit word size and more
	options.c_cflag &= ~CSTOPB;// Only one stop bit

	if (tcsetattr(serialFile, TCSANOW, &options) == -1)
	{
		outStream << "Failed to set serial port options:  "
			<< std::strerror(errno) << std::endl;
		CloseSerialFile();
		return false;
	}

	if (!SetBaud(normalBaud))
	{
		CloseSerialFile();
		return false;
	}

	return true;
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		CloseSerialFile (static)
//
// Description:		Decrements the counter and closes the file if the counter is zero.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::CloseSerialFile(void)
{
	assert(deviceCount > 0);
	assert(serialFile);

	deviceCount--;
	if (deviceCount > 0)
		return true;

	if (close(serialFile) == -1)
	{
		outStream << "Failed to close 1-wire serial port file:  "
			<< std::strerror(errno) << std::endl;
		return false;
	}

	return true;
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		SetBaud
//
// Description:		Sets the serial port speed to the specified rate.
//
// Input Arguments:
//		baud	= const speed_t&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::SetBaud(const speed_t& baud)
{
	assert(serialFile);

	struct termios options;
	if (tcgetattr(serialFile, &options) == -1)
	{
		outStream << "Failed to get serial port options in SetBaud():  "
			<< std::strerror(errno) << std::endl;
		return false;
	}

	if (cfsetspeed(&options, baud) == -1)
	{
		outStream << "Failed to set serial port speed:  "
			<< std::strerror(errno) << std::endl;
		return false;
	}

	if (tcsetattr(serialFile, TCSANOW, &options) == -1)
	{
		outStream << "Failed to set serial port options in SetBaud():  "
			<< std::strerror(errno) << std::endl;
		return false;
	}

	return true;
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		FamilyMatchesROM
//
// Description:		Checks that the ROM value is from the specified family.
//
// Input Arguments:
//		rom			= const std::string&
//		familyCode	= unsigned char
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if matches, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::FamilyMatchesROM(const std::string &rom,
	unsigned char familyCode)
{
	std::stringstream ss;
	ss << std::hex << rom.substr(rom.length() - 3);

	unsigned char romFamily;
	ss >> romFamily;

	return romFamily == familyCode;
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		FamilyMatchesROM
//
// Description:		Checks that the ROM value is from the specified family.
//
// Input Arguments:
//		familyCode	= unsigned char
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if matches, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::FamilyMatchesROM(unsigned char familyCode) const
{
	return FamilyMatchesROM(rom, familyCode);
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		CRCIsOK
//
// Description:		Checks that the specified string is valid (passes CRC).
//
// Input Arguments:
//		s	= const std::string&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if passes, false otherwise
//
//==========================================================================
bool UARTOneWireInterface::CRCIsOK(const std::string &s)
{
	return ComputeCRC(s) == 0;
}

//==========================================================================
// Class:			UARTOneWireInterface
// Function:		ComputeCRC
//
// Description:		Computes the CRC for the specified string.  The polynomial
//					used by Maxim (for all 1-wire devices) is X^8 + X^5 + X^4 + 1.
//
// Input Arguments:
//		s				= const std::string&
//		reverseInput	= const bool&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if passes, false otherwise
//
//==========================================================================
unsigned char UARTOneWireInterface::ComputeCRC(const std::string &s,
	const bool &reverseInput)
{
	assert(s.length() > 0);
	assert(s.length() % 2 == 0);// 8-bit CRC requires even number of hex characters

	unsigned int i, j;
	uint64_t value;
	std::stringstream ss;
	if (reverseInput)
	{
		for (i = 0; i < s.length() / 2; i++)
			ss << std::hex << s.substr(s.length() - (i + 1) * 2, 2);
	}
	else
		ss << std::hex << s;

	ss >> value;

	unsigned char crc(0);
	for (i = 0; i < s.length() / 2; i++)
	{
		j = (value ^ crc) & 0xFF;
		crc = 0;

		if(j & 1)
			crc ^= 0x5E;
		if(j & 2)
			crc ^= 0xBC;
		if(j & 4)
			crc ^= 0x61;
		if(j & 8)
			crc ^= 0xC2;
		if(j & 0x10)
			crc ^= 0x9D;
		if(j & 0x20)
			crc ^= 0x23;
		if(j & 0x40)
			crc ^= 0x46;
		if(j & 0x80)
			crc ^= 0x8C;

		value >>= 8;
	}

	return crc;
}
