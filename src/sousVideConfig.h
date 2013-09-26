// File:  sousVideConfig.h
// Date:  8/30/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Configuration options (to be read from file) for sous vide service.

#ifndef SOUS_VIDE_CONFIG_H_
#define SOUS_VIDE_CONFIG_H_

// Standard C++ headers
#include <string>
#include <iostream>
#include <map>
#include <cassert>

struct NetworkConfiguration
{
	unsigned short port;
};

struct IOConfiguration
{
	int pumpRelayPin;
	int heaterRelayPin;

	std::string sensorID;
};

struct ControllerConfiguration
{
	// Gains
	double kp;// [%/deg F]
	double ti;// [sec]
	double kd;// [sec]
	double kf;// [%-sec/deg F]

	// Derivative time constants
	double td;// [sec]
	double tf;// [sec]

	double plateauTolerance;// [deg F]
	double pwmFrequency;// [Hz]
};

struct InterlockConfiguration
{
	double maxSaturationTime;// [sec]
	double maxTemperature;// [deg F]
	double temperatureTolerance;// [deg F]

	double minErrorTime;// [sec]
};

struct SystemConfiguration
{
	InterlockConfiguration interlock;

	double idleFrequency;// [Hz]
	double activeFrequency;// [Hz]
	double statisticsTime;// [sec]

	double maxHeatingRate;// [deg F/sec]

	double maxAutoTuneTime;// [sec]
	double maxAutoTuneTemperatureRise;// [deg F]

	std::string temperaturePlotPath;
};

struct SousVideConfig
{
public:
	SousVideConfig(std::ostream &outStream = std::cout);

	bool ReadConfiguration(std::string fileName);

	// Configuration options to be read from file
	NetworkConfiguration network;
	IOConfiguration io;
	ControllerConfiguration controller;
	SystemConfiguration system;

private:
	static const std::string commentCharacter;
	std::ostream& outStream;

	template <typename T>
	void AddConfigItem(const std::string &key, T& data);
	void BuildConfigItems(void);
	void AssignDefaults(void);

	bool ConfigIsOK(void) const;
	bool NetworkConfigIsOK(void) const;
	bool IOConfigIsOK(void) const;
	bool ControllerConfigIsOK(void) const;
	bool InterlockConfigIsOK(void) const;
	bool SystemConfigIsOK(void) const;

	void SplitFieldFromData(const std::string &line, std::string &field, std::string &data);
	void ProcessConfigItem(const std::string &field, const std::string &data);
	bool ReadBooleanValue(const std::string &dataString) const;

	class ConfigItem
	{
	public:
		enum Type
		{
			TypeBool,
			TypeUnsignedChar,
			TypeChar,
			TypeUnsignedShort,
			TypeShort,
			TypeUnsignedInt,
			TypeInt,
			TypeUnsignedLong,
			TypeLong,
			TypeFloat,
			TypeDouble,
			TypeString
		};

		ConfigItem(bool &b) : type(TypeBool) { data.b = &b; };
		ConfigItem(unsigned char &uc) : type(TypeUnsignedChar) { data.uc = &uc; };
		ConfigItem(char &c) : type(TypeChar) { data.c = &c; };
		ConfigItem(unsigned short &us) : type(TypeUnsignedShort) { data.us = &us; };
		ConfigItem(short &s) : type(TypeShort) { data.s = &s; };
		ConfigItem(unsigned int &ui) : type(TypeUnsignedInt) { data.ui = &ui; };
		ConfigItem(int &i) : type(TypeInt) { data.i = &i; };
		ConfigItem(unsigned long &ul) : type(TypeUnsignedLong) { data.ul = &ul; };
		ConfigItem(long &l) : type(TypeLong) { data.l = &l; };
		ConfigItem(float &f) : type(TypeFloat) { data.f = &f; };
		ConfigItem(double &d) : type(TypeDouble) { data.d = &d; };
		ConfigItem(std::string &st) : type(TypeString) { this->st = &st; };

		void AssignValue(const std::string &data);

	private:
		const Type type;

		union
		{
			bool* b;
			unsigned char* uc;
			char* c;
			unsigned short* us;
			short* s;
			unsigned int* ui;
			int* i;
			unsigned long* ul;
			long* l;
			float* f;
			double* d;
		} data;

		std::string* st;
	};

	std::map<std::string, ConfigItem> configItems;
	std::map<void* const, std::string> keyMap;

	template <typename T>
	std::string GetKey(const T& i) const { return (*keyMap.find((void* const)&i)).second; }
};

template <typename T>
void SousVideConfig::AddConfigItem(const std::string &key, T& data)
{
	ConfigItem item(data);
	bool success = configItems.insert(std::make_pair(key, item)).second;
	assert(success);

	success = keyMap.insert(std::make_pair((void*)&data, key)).second;
	assert(success);
}

#endif// SOUS_VIDE_CONFIG_H_
