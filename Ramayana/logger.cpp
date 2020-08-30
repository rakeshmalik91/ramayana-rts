#include "stdafx.h"

#include "interface.h"

namespace ramayana {
	Logger Logger::logger;

	Logger::Logger() {
		string path = "log";
		createDir(path);
		time_t currentTime = time(NULL);
		string fname = ""
			+ toString(1900 + localtime(&currentTime)->tm_year, 4) + "-"
			+ toString(1 + localtime(&currentTime)->tm_mon, 2) + "-"
			+ toString(localtime(&currentTime)->tm_mday, 2) + "."
			+ toString(localtime(&currentTime)->tm_hour, 2) + "-"
			+ toString(localtime(&currentTime)->tm_min, 2) + "-"
			+ toString(localtime(&currentTime)->tm_sec, 2)
			+ ".log";
		log.open((path + "/" + fname).data(), ios_base::app);

		mutex.init();
	}
	void Logger::print(string msg, LogType type) {
		Synchronizer lock(mutex);
		string typeStr = "INFO";
		switch (type) {
		case LOG_SEVERE:
			typeStr = "SEVERE";
			break;
		}
		time_t currentTime = time(NULL);
		log << setw(6) << typeStr << " : "
			<< setprecision(2) << toString(localtime(&currentTime)->tm_mday, 2) << "-"
			<< setprecision(2) << toString(1 + localtime(&currentTime)->tm_mon, 2) << "-"
			<< setprecision(4) << toString(1900 + localtime(&currentTime)->tm_year, 4) << " : "
			<< setprecision(2) << toString(localtime(&currentTime)->tm_hour, 2) << "-"
			<< setprecision(2) << toString(localtime(&currentTime)->tm_min, 2) << "-"
			<< setprecision(2) << toString(localtime(&currentTime)->tm_sec, 2) << " : "
			<< "Thread-" << setprecision(10) << SDL_ThreadID() << " : "
			<< msg
			<< endl; 
	}

	Logger& getLogger() {
		return Logger::logger;
	}
}
