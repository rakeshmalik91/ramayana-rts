/*****************************************************************************************************
 * Subject                   : Exception class                                                       *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"

#include "exception.h"

namespace std {
	
	Exception::Exception(string msg) {
		this->msg=msg;
	}
	string Exception::getMessage() {
		return this->msg;
	}
	
	void showMessage(string msg, string title, bool toBeExited) {
	#ifdef _WINDOWS_
		wchar_t lmsg[1024];
		mbstowcs(lmsg, msg.data(), strlen(msg.data())+1);
		wchar_t ltitle[1024];
		mbstowcs(ltitle, title.data(), strlen(title.data())+1);
		MessageBox(NULL, (LPCWSTR)lmsg, (LPCWSTR)ltitle, MB_ICONWARNING);
	#endif
		cerr<<title.data()<<" : "<<msg.data()<<endl;
		if (toBeExited) {
			exit(1);
		}
	}
}
