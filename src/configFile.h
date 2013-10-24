// File:  configFile.h
// Date:  9/27/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Generic (abstract) config file class.

#ifndef CONFIG_FILE_H_
#define CONFIG_FILE_H_

// Standard C++ headers
#include <cstdio>
#include <string>
#include <iostream>
#include <map>
#include <sstream>
#include <cstring>
#include <cassert>
#include <fstream>
#include <vector>
#include <errno.h>

struct ConfigFile
{
public:
	ConfigFile(std::ostream &outStream = std::cout)
		: outStream(outStream) {};
	virtual ~ConfigFile() {};

	bool ReadConfiguration(std::string fileName);
	template <typename T>
	bool WriteConfiguration(std::string fileName,
		std::string field, T value);

protected:
	std::ostream& outStream;

	template <typename T>
	void AddConfigItem(const std::string &key, T& data);
	virtual void BuildConfigItems(void) = 0;
	virtual void AssignDefaults(void) = 0;

	virtual bool ConfigIsOK(void) = 0;

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
			TypeString,
			TypeStringVector
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
		ConfigItem(std::vector<std::string> &sv) : type(TypeStringVector) { this->sv = &sv; };

		void AssignValue(const std::string &data);

	private:
		const Type type;

		bool InterpretBooleanData(const std::string &dataString) const;

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
		std::vector<std::string>* sv;
	};

	template <typename T>
	std::string GetKey(const T& i) const { return (*keyMap.find((void* const)&i)).second; }

private:
	static const std::string commentCharacter;

	void StripCarriageReturn(std::string &s) const;
	void SplitFieldFromData(const std::string &line, std::string &field, std::string &data);
	void ProcessConfigItem(const std::string &field, const std::string &data);

	std::map<std::string, ConfigItem> configItems;
	std::map<void* const, std::string> keyMap;
};

//==========================================================================
// Class:			ConfigFile
// Function:		AddConfigItem
//
// Description:		Adds the specified field key and data reference to the list.
//
// Input Arguments:
//		key		= const std::string&
//		data	= T&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
template <typename T>
void ConfigFile::AddConfigItem(const std::string &key, T& data)
{
	ConfigItem item(data);
	bool success = configItems.insert(std::make_pair(key, item)).second;
	assert(success);

	success = keyMap.insert(std::make_pair((void*)&data, key)).second;
	assert(success);
}

//==========================================================================
// Class:			ConfigFile
// Function:		WriteConfiguration
//
// Description:		Writes the specified field to the config file.  Maintains
//					existing whitespace, comments, formatting, etc.
//
// Input Arguments:
//		fileName	= std::string
//		field		= std::string
//		value		= T
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for success, false otherwise
//
//==========================================================================
template <typename T>
bool ConfigFile::WriteConfiguration(std::string fileName,
	std::string field, T value)
{
	std::ifstream inFile(fileName.c_str(), std::ios::in);
	if (!inFile.is_open() || !inFile.good())
	{
		outStream << "Failed to open '" << fileName << "'" << std::endl;
		return false;
	}

	std::string tempFileName("tempConfigFile");
	std::ofstream outFile(tempFileName.c_str(), std::ios::out);
	if (!outFile.is_open() || !outFile.good())
	{
		outStream << "Failed to open '" << tempFileName << "'" << std::endl;
		return false;
	}

	std::string line, comment, currentField, data;
	size_t inLineComment;

	bool written(false);
	while (std::getline(inFile, line))
	{
		if (!written && !line.empty() &&
			commentCharacter.compare(line.substr(0,1)) != 0)
		{
			inLineComment = line.find(commentCharacter);
			if (inLineComment != std::string::npos)
			{
				comment = line.substr(inLineComment);
				line = line.substr(0, inLineComment);
			}
			else
				comment.clear();

			SplitFieldFromData(line, currentField, data);
			if (currentField.compare(field) == 0)
			{
				std::stringstream ss;
				ss << field << " = " << value;
				line = ss.str();
				written = true;
			}
		}

		outFile << line << comment << std::endl;
	}

	if (!written)
		outFile << field << " = " << value << std::endl;

	inFile.close();
	outFile.close();
	if (std::remove(fileName.c_str()) == -1)
	{
		outStream << "Failed to delete '" << fileName
			<< "':  " << std::strerror(errno) << std::endl;
		return false;
	}

	if (std::rename(tempFileName.c_str(), fileName.c_str()) == -1)
	{
		outStream << "Failed to rename '" << tempFileName
			<< "' to '" << fileName << "':  " << std::strerror(errno) << std::endl;
		return false;
	}

	return true;
}

#endif// CONFIG_FILE_H_
