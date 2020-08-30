#ifndef __EXCEPTION_H
#define __EXCEPTION_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

namespace std {

	class Exception {
		string msg;
	public:
		Exception() {}
		Exception(string);
		string getMessage();
	};
}

#endif