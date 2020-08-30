
#include "stdafx.h"

#include "thread.h"

namespace thread {
	Lock::Lock()
		: mutex(NULL) {}
	void Lock::init() {
		if (mutex != NULL) {
			throw SynchronizationException("Lock already initialized");
		}
		mutex = SDL_CreateMutex();
		mapMutex = SDL_CreateMutex();
	}
	void Lock::lock() {
		if (mutex == NULL) {
			throw SynchronizationException("Lock not initialized");
		}

		SDL_LockMutex(mapMutex);
		int threadID = SDL_ThreadID();
		if (locked.find(threadID) == locked.end()) {
			locked.insert(pair<int, int>(threadID, 1));
		} else {
			locked[threadID]++;
			if (locked[threadID] > 1) {
				SDL_UnlockMutex(mapMutex);
				return;
			}
		}
		SDL_UnlockMutex(mapMutex);

		SDL_LockMutex(mutex);
	}
	void Lock::unlock() {
		if (mutex == NULL) {
			throw SynchronizationException("Lock not initialized");
		}

		SDL_LockMutex(mapMutex);
		int threadID = SDL_ThreadID();
		if (locked.find(threadID) == locked.end() || locked[threadID] <= 0) {
			SDL_UnlockMutex(mapMutex);
			throw SynchronizationException("Tried to unlock while not locked");
		}
		locked[threadID]--;
		SDL_UnlockMutex(mapMutex);

		SDL_UnlockMutex(mutex);
	}
	Lock::~Lock() {
		if (mutex != NULL) {
			SDL_DestroyMutex(mutex);
			SDL_DestroyMutex(mapMutex);
			mutex = NULL;
			locked.clear();
		}
	}


	Synchronizer::Synchronizer(Lock &lock)
		: lock(lock) {
		lock.lock();
	}
	Synchronizer::~Synchronizer() {
		lock.unlock();
	}
}