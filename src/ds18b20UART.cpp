// File:  ds18b20UART.cpp
// Date:  10/8/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  DS18B20 temperature sensor object, using UART to implement the 1-wire
//        communication interface.  See
//        http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf for more
//        information.

// Standard C++ headers
#include <cmath>
#include <sstream>
#include <cassert>

// *nix standard headers
#include <unistd.h>

// Local headers
#include "ds18b20UART.h"

//==========================================================================
// Class:			DS18B20UART
// Function:		Constant Definitions
//
// Description:		Constant definitions for the DS18B20UART class.
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
const unsigned char DS18B20UART::familyCode = 0x28;
const unsigned char DS18B20UART::convertTCommand = 0x44;
const unsigned char DS18B20UART::readScratchPadCommand = 0xBE;
const unsigned char DS18B20UART::writeScratchPadCommand = 0x4E;
const unsigned char DS18B20UART::copyScratchPadCommand = 0x48;
const unsigned char DS18B20UART::recallEECommand = 0xB8;
const unsigned char DS18B20UART::readPowerSupplyCommand = 0xB4;

//==========================================================================
// Class:			DS18B20UART
// Function:		DS18B20UART
//
// Description:		Constructor for DS18B20UART class.
//
// Input Arguments:
//		rom			= cosnt std::string& indicating the 64-bit ROM code used
//					  to identify slave devices
//		outStream	= std::ostream& to direct output to
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
DS18B20UART::DS18B20UART(const std::string &rom, std::ostream& outStream)
	: UARTOneWireInterface(rom)
{
//	this->outStream = outStream;// TODO:  Fix this

	if (!FamilyMatchesROM(familyCode))
	{
		outStream << "Specified ROM (" << rom << ") does not match family code (0x"
			<< std::hex << (int)familyCode << ")" << std::endl;
		return;
	}

	ReadPowerSupply(powerMode);
	ReadScratchPad();
}

//==========================================================================
// Class:			DS18B20UART
// Function:		ReadScratchPad
//
// Description:		Reads the scratch pad contents from the DS18B20 sensor.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool DS18B20UART::ReadScratchPad(void)
{
	if (!Select() || !Write(readScratchPadCommand))
		return false;

	const unsigned int scratchPadSize(9);
	unsigned char b[scratchPadSize];
	unsigned int i;
	std::stringstream crcStream;
	for (i = 0; i < scratchPadSize; i++)
	{
		if (!Read(b[i]))
			return false;
		crcStream << b[i];
	}

	temperature = BytesToDouble(b[0], b[1]);
	alarmTemperature = BytesToDouble(b[2], b[3]);
	resolution = ByteToResolution(b[4]);
	// Byte 5 is reserved and will always be 0xFF -> check it?
	// Byte 6 is reserved and can vary
	// Byte 7 is reserved and will always be 0x10 -> check it?

	if (!CRCIsOK(crcStream.str()))
	{
		outStream << "Error:  Scratch pad read invalid (CRC)" << std::endl;
		return false;
	}

	return true;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		WriteScratchPad
//
// Description:		Writes new scrath pad contents to the DS18B20.
//
// Input Arguments:
//		alarmTemperature	= const double& [deg C]
//
// Output Arguments:
//		None
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool DS18B20UART::WriteScratchPad(const double &alarmTemperature) const
{
	return WriteScratchPad(alarmTemperature, resolution);
}

//==========================================================================
// Class:			DS18B20UART
// Function:		WriteScratchPad
//
// Description:		Writes new scrath pad contents to the DS18B20.
//
// Input Arguments:
//		resolution	= const TemperatureResolution&
//
// Output Arguments:
//		None
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool DS18B20UART::WriteScratchPad(const TemperatureResolution &resolution) const
{
	return WriteScratchPad(alarmTemperature, resolution);
}

//==========================================================================
// Class:			DS18B20UART
// Function:		WriteScratchPad
//
// Description:		Writes new scrath pad contents to the DS18B20.
//
// Input Arguments:
//		alarmTemperature	= const double& [deg C]
//		resolution			= const TemperatureResolution&
//
// Output Arguments:
//		None
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool DS18B20UART::WriteScratchPad(const double &alarmTemperature,
	const TemperatureResolution &resolution) const
{
	unsigned char tL, tH;
	DoubleToBytes(alarmTemperature, tL, tH);

	if (!Select() ||
		!Write(tH) ||
		!Write(tL) ||
		!Write(ResolutionToByte(resolution)))
		return false;

	return true;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		ReadPowerSupply
//
// Description:		Requests DS18B20 to signal power supply mode back to master.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		powerMode	= PowerSupply&
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool DS18B20UART::ReadPowerSupply(PowerSupply& powerMode) const
{
	unsigned char response;
	if (!Select() ||
		!Write(readPowerSupplyCommand) ||
		!Read(response))
		return false;

	if (response == 0)
		powerMode = PowerSupplyParasitic;
	else
		powerMode = PowerSupplyExternal;

	return true;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		ConvertTemperature
//
// Description:		Signals DS18B20 to read current temperature and store result
//					in scratch pad.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool DS18B20UART::ConvertTemperature(void) const
{
	if (!Select() ||
		!Write(convertTCommand))
		return false;

	if (powerMode == PowerSupplyExternal)
	{
		if (!WaitForReadOne())
			return false;
	}
	else
		usleep(GetConversionTime());

	return true;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		SaveConfigurationToEEPROM
//
// Description:		Signals DS18B20 to copy scratch pad contents to EEPROM.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool DS18B20UART::SaveConfigurationToEEPROM(void) const
{
	if (!Select() ||
		!Write(copyScratchPadCommand))
		return false;

	if (powerMode == PowerSupplyParasitic)
		usleep(10000);

	return true;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		LoadConfigurationFromEEPROM
//
// Description:		Signals DS18B20 to copy EEPROM contents to scratch pad.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool DS18B20UART::LoadConfigurationFromEEPROM(void) const
{
	if (!Select() ||
		!Write(recallEECommand))
		return false;

	if (!WaitForReadOne())
		return false;

	return true;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		WaitForReadOne
//
// Description:		Loops, issuing read slots to the DS18B20 until it returns
//					a one.  Useful for waiting until conversions/measurements
//					are complete.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		true for success, false otherwise
//
//==========================================================================
bool DS18B20UART::WaitForReadOne(void) const
{
	unsigned char response;
	do
	{
		if (!Read(response))
			return false;

		// Do we need a small sleep here to prevent eating up too much CPU?

	} while (response != 1);

	return true;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		ResolutionToByte
//
// Description:		Returns the proper configuration byte for the specified
//					resolution.
//
// Input Arguments:
//		resolution	= const TemperatureResolution&
//
// Output Arguments:
//		None
//
// Return Value:
//		unsigned char containing configuration byte (scratch pad byte 4)
//
//==========================================================================
unsigned char DS18B20UART::ResolutionToByte(const TemperatureResolution& resolution) const
{
	// From page 8 of DS18B20 manual
	//  BIT 7    BIT 6    BIT 5    BIT 4    BIT 3    BIT 2    BIT 1    BIT 0
	//    0       R1       R0        1        1        1        1        1
	//
	// R1    R0    Resolution    Max. Conversion Time
	// 0     0        9-bit           93.75 msec
	// 0     1       10-bit           187.5 msec
	// 1     0       11-bit             375 msec
	// 1     1       12-bit             750 msec

	unsigned char r = 0x7F;// = 0b01111111
	if (resolution == Resolution9Bit || resolution == Resolution11Bit)
		r &= ~32;// R0 (bit 5) -> 0
	if (resolution == Resolution9Bit || resolution == Resolution10Bit)
		r &= ~64;// R1 (bit 6) -> 0

	return r;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		ByteToResolution
//
// Description:		Returns the resolution as specified by the configuration byte.
//
// Input Arguments:
//		b	= const unsigned char&
//
// Output Arguments:
//		None
//
// Return Value:
//		TemperatureResolution
//
//==========================================================================
DS18B20UART::TemperatureResolution DS18B20UART::ByteToResolution(const unsigned char &b) const
{
	if (b == 31)
		return Resolution9Bit;
	else if (b == 63)
		return Resolution10Bit;
	else if (b == 95)
		return Resolution11Bit;
	else if (b == 127)
		return Resolution12Bit;

	return ResolutionInvalid;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		BytesToDouble
//
// Description:		Converts a value (double) to two bytes.
//
// Input Arguments:
//		tL	= const unsigned char& (LSB)
//		tH	= const unsigned char& (MSB)
//
// Output Arguments:
//		None
//
// Return Value:
//		double, representing temperature [deg C]
//
//==========================================================================
double DS18B20UART::BytesToDouble(const unsigned char &tL, const unsigned char &tH) const
{
	// From page 4 in DS18B20 datasheet:
	//      BIT 7    BIT 6    BIT 5    BIT 4    BIT 3    BIT 2    BIT 1    BIT 0
	// LSB   2^3      2^2      2^1      2^0      2^-1     2^-2     2^-3     2^-4
	//      BIT 15   BIT 14   BIT 13   BIT 12   BIT 11   BIT 10   BIT 9    BIT 8
	// MSB    S        S        S        S        S       2^6      2^5      2^4
	// where S indicates the sign

	int maskedTh = tH & 0x07;
	int combined = tL + (maskedTh << 8);
	double temperature(combined * 0.0625);// [deg C]

	if ((tH & 0xF8) == 0xF8)
		temperature -= 128.0;

	return temperature;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		DoubleToBytes
//
// Description:		Converts a value (double) to two bytes.
//
// Input Arguments:
//		d	= const double&
//
// Output Arguments:
//		tL	= unsigned char&
//		tH	= unsigned char&
//
// Return Value:
//		None
//
//==========================================================================
void DS18B20UART::DoubleToBytes(const double &d, unsigned char &tL, unsigned char &tH) const
{
	double v(d);
	if (d < 0.0)
		v += 128.0;
	int combined = (int)floor(v * 16.0 + 0.5);

	tL = combined & 0xFF;
	tH = (combined >> 8) & 0xFF;

	if (d >= 0.0)
		tH &= 0x07;
	else
		tH |= 0xF8;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		GetConversionTime
//
// Description:		Returns the conversion time for the current resolution.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		unsigned int specifying the conversion time in usec
//
//==========================================================================
unsigned int DS18B20UART::GetConversionTime(void) const
{
	if (resolution == Resolution9Bit)
		return 93750;
	else if (resolution == Resolution10Bit)
		return 187500;
	else if (resolution == Resolution11Bit)
		return 375000;
	else if (resolution == Resolution12Bit)
		return 750000;

	assert(false);
	return 0;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		SearchROMs
//
// Description:		Searches for connected ROMs that are from the DS18B20 family.
//
// Input Arguments:
//		rom	= const std::string&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true success, false otherwise
//
//==========================================================================
bool DS18B20UART::SearchROMs(std::vector<std::string>& roms)
{
	if (!UARTOneWireInterface::SearchROMs(roms))
		return false;

	unsigned int i;
	for (i = 0; i < roms.size(); i++)
	{
		if (!ROMIsInFamily(roms[i]))
		{
			roms.erase(roms.begin() + i);
			i--;
		}
	}

	return true;
}

//==========================================================================
// Class:			DS18B20UART
// Function:		ROMIsInFamily
//
// Description:		Indicates whether or not the specified ROM is in the family.
//
// Input Arguments:
//		rom	= const std::string&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the ROM is a member of the family, false otherwise
//
//==========================================================================
bool DS18B20UART::ROMIsInFamily(const std::string &rom)
{
	return FamilyMatchesROM(rom, familyCode);
}
