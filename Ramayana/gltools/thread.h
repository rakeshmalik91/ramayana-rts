#ifndef __THREAD_H
#define __THREAD_H

#include <map>

#include <SDL/SDL_thread.h>

#include "exception.h"

using namespace std;

namespace thread {

	class SynchronizationException : Exception {
	public:
		SynchronizationException() : Exception() {}
		SynchronizationException(string msg) : Exception(msg) {}
	};

	// a lock class that checks if it has already locked on current 
	class Lock {
		SDL_mutex *mutex;
		map<int, int> locked;
		SDL_mutex *mapMutex;
	public:
		Lock();
		void init();
		void lock();
		void unlock();
		~Lock();
	};

	// A block synchronizer that works on a lock
	// to be used as following : 
	//	some_func() {
	//		normal_works();
	//		{
	//			Synchronizer sync(some_lock);
	//			do_critical_works();
	//		}
	//		other_normal_works();
	//	}
	class Synchronizer {
		Lock &lock;
	public:
		Synchronizer(Lock&);
		~Synchronizer();
	};
}

#endif
