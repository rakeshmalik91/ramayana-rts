#ifndef __STRING_H
#define __STRING_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

namespace std {

	string getFirstField(string &s, char delim);
	string leftTrim(string s);
	string rightTrim(string s);
	string trim(string s);
	string sqeeze(string s);
	bool startsWith(string big, string small);
	bool endsWith(string big, string small);
	string toLower(string s);
	string toUpper(string s);
	string toString(unsigned int i, int padding = 0);
	string toString(int i, int padding = 0);
	string toString(double d, int precision = 0);
	string toString(bool b);
	string toString(string s);
	int toInt(string s);
	float toFloat(string s);
	bool toBool(string s);
	string operator+(string s, int i);
	string operator+(int i, string s);
	string operator+(string s, float i);
	string operator+(float i, string s);
	string operator+(string s, bool i);
	string operator+(bool i, string s);
	string replace(string s, char c1, char c2);
	string timeAsString(unsigned int t);
	vector<string> split(string str, char delim = ' ', int limit = 0);
}

#endif