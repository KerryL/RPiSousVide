// File:  configFile.h
// Date:  9/27/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Generic (abstract) config file class.

#ifndef CONFIG_FILE_H_
#define CONFIG_FILE_H_

// Standard C++ headers
#include <string>
#include <iostream>
#include <map>
#include <cassert>

struct ConfigFile
{
public:
	ConfigFile(std::ostream &outStream = std::cout)
		: outStream(outStream) {};
	virtual ~ConfigFile() {};

	bool ReadConfiguration(std::string fileName);
	bool WriteConfiguration(std::string fileName,
		std::string field, std::string value);

protected:
	std::ostream& outStream;

	template <typename T>
	void AddConfigItem(const std::string &key, T& data);
	virtual void BuildConfigItems(void) = 0;
	virtual void AssignDefaults(void) = 0;

	virtual bool ConfigIsOK(void) const = 0;

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

	template <typename T>
	std::string GetKey(const T& i) const { return (*keyMap.find((void* const)&i)).second; }

private:
	static const std::string commentCharacter;

	void SplitFieldFromData(const std::string &line, std::string &field, std::string &data);
	void ProcessConfigItem(const std::string &field, const std::string &data);
	bool ReadBooleanValue(const std::string &dataString) const;

	std::map<std::string, ConfigItem> configItems;
	std::map<void* const, std::string> keyMap;
};

template <typename T>
void ConfigFile::AddConfigItem(const std::string &key, T& data)
{
	ConfigItem item(data);
	bool success = configItems.insert(std::make_pair(key, item)).second;
	assert(success);

	success = keyMap.insert(std::make_pair((void*)&data, key)).second;
	assert(success);
}

#endif// CONFIG_FILE_H_
