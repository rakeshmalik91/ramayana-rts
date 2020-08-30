/*****************************************************************************************************
 * Subject                   : String handling functions                                             *
 * Author                    : Rakesh Malik                                                          *
 *****************************************************************************************************/

#include "stdafx.h"

namespace std {

	//Cuts and returns first substring of "s" delimited by "delim"
	string getFirstField(string &s, char delim) { 
		char f[1000]="";
		try {
			int i;
			for(i=0; i<s.size(); i++) {
				if(s[i]==delim) {
					s[i]='\0';
					i++;
					break;
				} else {
					f[i]=s[i];
				}
			}
			s=s.substr(i);
		} catch(...) {}
		return (string)f;
	}
	
	string leftTrim(string s) {
		int nSpace=0;
		while(nSpace<s.size() && isspace(s[nSpace]))
			nSpace++;
		return s.substr(nSpace);
	}
	string rightTrim(string s) {
		int nSpace=0;
		while(nSpace<s.size() && isspace(s[s.size()-nSpace-1]))
			nSpace++;
		return s.substr(0, s.size()-nSpace);
	}
	string trim(string s) {
		return rightTrim(leftTrim(s));
	}
	
	string sqeeze(string s) {
		string s1;
		char c = '\0';
		for(int i = 0; i < s.length(); i++) {
			if(!isspace(s[i])) {
				if(isspace(c))
					s1+=c;
				s1+=s[i];
			}
			c=s[i];
		}
		return s1;
	}

	bool startsWith(string big, string small) {
		return big.length()>=small.length() && big.substr(0, small.length())==small;
	}
	bool endsWith(string big, string small) {
		return big.length()>=small.length() && big.substr(big.length()-small.length(), small.length())==small;
	}
	
	string toLower(string s) {
		for(int i=0; i<s.length(); i++)
			s[i]=tolower(s[i]);
		return s;
	}
	string toUpper(string s) {
		for(int i=0; i<s.length(); i++)
			s[i]=toupper(s[i]);
		return s;
	}
	
	string toString(unsigned int i, int padding) {
		char s[50];
		if(padding==0)
			sprintf(s, "%u", i);
		else
			sprintf(s, "%.*u", padding, i);
		return (string)s;
	}
	string toString(int i, int padding) {
		char s[50];
		if(padding==0)
			sprintf(s, "%d", i);
		else
			sprintf(s, "%.*d", padding, i);
		return (string)s;
	}	
	string toString(double d, int precision) {
		char s[50];
		if(precision==0)
			sprintf(s, "%g", d);
		else
			sprintf(s, "%.*f", precision, d);
		return (string)s;
	}
	string toString(bool b) {
		return b?"true":"false";
	}
	string toString(string s) {
		return s;
	}
	
	int toInt(string s) {
		return atoi(s.data());
	}
	float toFloat(string s) {
		return atof(s.data());
	}
	bool toBool(string s) {
		s = trim(s);
		return s=="false"?false:s=="0"?false:true;
	}
	
	string operator+(string s, int i) {
		return s+toString(i);
	}
	string operator+(int i, string s) {
		return toString(i)+s;
	}
	string operator+(string s, float i) {
		return s+toString(i);
	}
	string operator+(float i, string s) {
		return toString(i)+s;
	}
	string operator+(string s, bool i) {
		return s+toString(i);
	}
	string operator+(bool i, string s) {
		return toString(i)+s;
	}
	
	string replace(string s, char c1, char c2) {
		for(int i=0; i<s.size(); i++)
			if(s[i]==c1)
				s[i]=c2;
		return s;
	}
	
	string timeAsString(unsigned int t) {
	int ms=t%1000;
	int s=(t/1000)%60;
	int m=(t/60000)%60;
	int h=(t/3600000)%24;
	int d=(t/86400000)%7;
	int w=(t/604800000);

	string str=toString(ms, 3)+" ms";
	if(w>0 || d>0 || h>0 || m>0 || s>0) str=toString(s, 2)+" sec "+str;
	if(w>0 || d>0 || h>0 || m>0) str=toString(m, 2)+" min "+str;
	if(w>0 || d>0 || h>0) str=toString(h, 2)+" hr "+str;
	if(w>0 || d>0) str=toString(d)+" day "+str;
	if(w>0) str=toString(w)+" week "+str;
	return str;
}

	vector<string> split(string str, char delim, int limit) {
		vector<string> list;
		list.push_back("");
		for(int i = 0; i < str.length(); i++) {
			if(str[i] == delim) {
				if(limit<=0 || list.size()<limit)
					list.push_back("");
				else
					list.back() += str[i];
			} else {
				list.back() += str[i];
			}
		}
		return list;
	}

}
