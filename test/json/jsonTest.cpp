// File:  jsonTest.cpp
// Date:  9/21/2013
// Auth:  K. Loux
// Copy:  (c) Copyright 2013
// Desc:  Test application (or more of a learning tool) for cJSON.

// Standard C++ headers
#include <cstdlib>
#include <iostream>
#include <string>

// Local headers
#include "cJSON.h"

using namespace std;

// Application entry point
int main(int, char *[])
{
	// Encode some data
	double value1(203.3), value2(-29.21);
	int value3(93.2);
	std::string s("this is a test string");

	cout << "Before encoding:" << endl;
	cout << "value1 = " << value1 << endl;
	cout << "value2 = " << value2 << endl;
	cout << "value3 = " << value3 << endl;
	cout << "s = " << s << endl;

	cJSON *root = cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "value1", value1);
	cJSON_AddNumberToObject(root, "value2", value2);
	cJSON_AddNumberToObject(root, "value3", value3);
	cJSON_AddStringToObject(root, "s", s.c_str());

	string jsonString(cJSON_Print(root));
	cJSON_Delete(root);

	cout << endl;
	cout << jsonString << endl;
	cout << endl;

	double v1, v2;
	int v3;
	string ds;
	root = cJSON_Parse(jsonString.c_str());
	v1 = cJSON_GetObjectItem(root, "value1")->valuedouble;
	v2 = cJSON_GetObjectItem(root, "value2")->valuedouble;
	v3 = cJSON_GetObjectItem(root, "value3")->valueint;
	ds.assign(cJSON_GetObjectItem(root, "s")->valuestring);

	cout << "After decoding:" << endl;
	cout << "value1 = " << v1 << endl;
	cout << "value2 = " << v2 << endl;
	cout << "value3 = " << v3 << endl;
	cout << "s = " << ds << endl;

	cout << endl;
	cout << "This is what you get if you request a non-existent field:" << endl;
	cout << cJSON_GetObjectItem(root, "non-existent") << endl;
	cout << "vs. requesting a good field:" << endl;
	cout << cJSON_GetObjectItem(root, "value1") << endl;

	cJSON_Delete(root);

	return 0;
}
