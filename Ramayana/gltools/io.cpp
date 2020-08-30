/*****************************************************************************************************
 * Subject                   : File                                                                  *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"

#include "io.h"

using namespace std;

namespace std {

	//Returns file name/path removing the extension
	string removeExtension(string file) {
		int i;
		for(i=file.length()-1; i>=0; i--)
			if(file[i]=='.')
				break;
		if(i>0)
			return file.substr(0, i);
		else
			return file;
	}
	
	//Returns extension from a file
	string getExtension(string file) {
		int i;
		for(i=file.length()-1; i>=0; i--)
			if(file[i]=='.')
				break;
		if(i>0)
			return file.substr(i+1, file.length());
		else
			return "";
	}
	
	//Returns only directory path from a file/directory path
	string getFilePath(string name) {
		for(int i=name.length()-1; i>=0; i--)
			if(name[i]=='\\' || name[i]=='/')
				return name.substr(0, i);
		return name;
	}
	string removeFilePath(string name) {
		for(int i=name.length()-1; i>=0; i--)
			if(name[i]=='\\' || name[i]=='/')
				return name.substr(i+1);
		return name;
	}
	
	//Return whether given file exists
	bool fileExists(string fname) {
		ifstream file(fname.data());
		bool exists = !file.fail();
		if(exists) file.close();
		return exists;
	}
	
	//Returns files in directory
	vector<string> getFiles(string dirname) {
		DIR* dir=opendir(dirname.data());
		if(dir==NULL)
			throw DirectoryNotFoundException(dirname);
		vector<string> filenames;
		for(dirent *d; (d=readdir(dir))!=NULL; ) {
			if(string(d->d_name)!="." && string(d->d_name)!="..")
				filenames.push_back(string(d->d_name));
		}
		closedir(dir);
		return filenames;
	}
	vector<string> getFilePaths(string dirname) {
		DIR* dir=opendir(dirname.data());
		if(dir==NULL)
			throw DirectoryNotFoundException(dirname);
		vector<string> filenames;
		for(dirent *d; (d=readdir(dir))!=NULL; ) {
			if(string(d->d_name)!="." && string(d->d_name)!="..")
				filenames.push_back(dirname+"/"+string(d->d_name));
		}
		closedir(dir);
		return filenames;
	}
	
	string readTextFile(const char *filename) {
		string buffer;
	    ifstream file(filename, ios::binary);
		if (file.fail())
			throw FileNotFoundException(filename);
	    if (file.is_open()) {
	        file.seekg(0, ios::end);
	        ifstream::pos_type fileSize = file.tellg();
	        buffer.resize(static_cast<unsigned int>(fileSize));
	        file.seekg(0, ios::beg);
	        file.read(&buffer[0], fileSize);
	    }
		return buffer;
	}

	void writeStringAsBinary(ostream &out, string str) {
		//string length
		int len = str.size();
		out.write((const char*)&len, sizeof(len));
		//string data
		out.write(&str[0], len);
	}
	string readStringAsBinary(istream &in) {
		//string length
		int len = 0;
		in.read((char*)&len, sizeof(len));
		//string data
		string str;
		str.resize(len, '\0');
		in.read(&str[0], len);
		return str;
	}

	void createDir(string dir) {
#if defined _MSC_VER
		_mkdir(dir.data());
#elif defined __GNUC__
		mkdir(dir.data(), 0777);
#endif
	}
}