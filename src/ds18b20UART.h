// File:  ds18b20UART.h
// Date:  10/8/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  DS18B20 temperature sensor object, using UART to implement the 1-wire
//        communication interface.  See
//        http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf for more
//        information.

#ifndef DS18B20_UART_H_
#define DS18B20_UART_H_

// Standard C++ headers
#include <iostream>

// Local headers
#include "uartOneWireInterface.h"

class DS18B20UART : public UARTOneWireInterface
{
public:
	DS18B20UART(const std::string &rom, std::ostream& outStream = std::cout);

	enum TemperatureResolution
	{
		Resolution9Bit,// tConv = 93.75 msec; resolution = 0.5 deg C
		Resolution10Bit,// tConv = 187.5 msec; resolution = 0.25 deg C
		Resolution11Bit,// tConv = 375 msec; resolution = 0.125 deg C
		Resolution12Bit,// tConv = 750 msec; resolution = 0.0625 deg C
		ResolutionInvalid
	};

	enum PowerSupply
	{
		PowerSupplyExternal,
		PowerSupplyParasitic
	};

	bool ReadScratchPad(void);
	bool WriteScratchPad(const double &alarmTemperature) const;
	bool WriteScratchPad(const TemperatureResolution &resolution) const;
	bool WriteScratchPad(const double &alarmTemperature,
		const TemperatureResolution &resolution) const;

	bool ReadPowerSupply(PowerSupply& powerMode) const;
	bool ConvertTemperature(void) const;
	static bool BroadcastConvertTemperature(void);
	bool WaitForConversionComplete(void) const;

	bool SaveConfigurationToEEPROM(void) const;
	bool LoadConfigurationFromEEPROM(void) const;

	double GetTemperature(void) const { return temperature; };// [deg C]
	double GetAlarmTemperature(void) const { return alarmTemperature; };// [deg C]
	TemperatureResolution GetResolution(void) const { return resolution; };

	static bool SearchROMs(std::vector<std::string>& roms);
	static bool ROMIsInFamily(const std::string &rom);

private:
	static const unsigned char familyCode;
	static const unsigned char convertTCommand;
	static const unsigned char writeScratchPadCommand;
	static const unsigned char readScratchPadCommand;
	static const unsigned char copyScratchPadCommand;
	static const unsigned char recallEECommand;
	static const unsigned char readPowerSupplyCommand;

	PowerSupply powerMode;
	double temperature;
	double alarmTemperature;
	TemperatureResolution resolution;

	unsigned char ResolutionToByte(const TemperatureResolution& resolution) const;
	TemperatureResolution ByteToResolution(const unsigned char &b) const;
	double BytesToDouble(const unsigned char &tL, const unsigned char &tH) const;
	void DoubleToBytes(const double &d, unsigned char &tL, unsigned char &tH) const;

	unsigned int GetConversionTime(void) const;
	bool WaitForReadOne(void) const;
};

#endif// DS18B20_UART_H_
