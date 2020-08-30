#ifndef __IO_H
#define __IO_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "exception.h"

namespace std {
	
	void showMessage(string msg, string title, bool exit=false);
	
	class FileNotFoundException : public Exception {
	public:
		FileNotFoundException(string fname) : Exception("File \""+fname+"\" not found"){}
	};
	class DirectoryNotFoundException : public Exception {
	public:
		DirectoryNotFoundException(string fname) : Exception("Directory \""+fname+"\" not found"){}
	};
	class FileReadException : public Exception {
	public:
		FileReadException(string fname) : Exception("File \""+fname+"\" not found"){}
	};
	class FileWriteException : public Exception {
	public:
		FileWriteException(string fname) : Exception("File \""+fname+"\" not found"){}
	};
	
	string removeExtension(string file);
	string getExtension(string file);
	string getFilePath(string name);
	string removeFilePath(string name);
	bool fileExists(string fname);
	vector<string> getFiles(string dirname);
	vector<string> getFilePaths(string dirname);
	string readTextFile(const char *filename);

	void createDir(string dir);

	void writeStringAsBinary(ostream &out, string str);
	string readStringAsBinary(istream &in);
}

#endif
