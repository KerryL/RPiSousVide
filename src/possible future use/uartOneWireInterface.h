// File:  uartOneWireInterface.h
// Date:  10/8/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  C++ wrapper around UART interface for communicating with 1-wire
//        peripherals.  See
//        http://www.maximintegrated.com/app-notes/index.mvp/id/214 for more
//        information.

#ifndef UART_ONE_WIRE_INTERFACE_H_
#define UART_ONE_WIRE_INTERFACE_H_

// Standard C++ headers
#include <string>
#include <vector>
#include <ostream>

// *nix standard headers
#include <termios.h>

class UARTOneWireInterface
{
public:
	UARTOneWireInterface(const std::string &rom);
	virtual ~UARTOneWireInterface();

	static bool ResetAndPresenceDetect(void);
	static bool SearchROMs(std::vector<std::string>& roms);
	static bool AlarmSearch(std::vector<std::string>& roms);

	bool Select(void) const;

	static bool Write(const unsigned char &c);
	static bool Write(const std::string &c);
	static bool Read(unsigned char &c);

	static std::string ttyFile;

private:
	static const speed_t normalBaud;
	static const speed_t resetBaud;

	static unsigned int deviceCount;
	static int serialFile;

	static bool OpenSerialFile(void);
	static bool CloseSerialFile(void);

	static bool FindAllDevicesWithCommand(const unsigned char &command,
		std::vector<std::string>& roms);

	const std::string rom;

	static bool SetBaud(const speed_t& baud);

	static unsigned char ComputeCRC(const std::string &s, const bool &reverseInput = false);

protected:
	static const unsigned char searchROMCommand;
	//static const unsigned char readROMCommand;// search ROM is preferred
	static const unsigned char matchROMCommand;
	static const unsigned char skipROMCommand;
	static const unsigned char alarmSearchCommand;

	static std::ostream& outStream;
	static bool FamilyMatchesROM(const std::string &rom, unsigned char familyCode);
	bool FamilyMatchesROM(unsigned char familyCode) const;

	static bool CRCIsOK(const std::string &s);
};

#endif
