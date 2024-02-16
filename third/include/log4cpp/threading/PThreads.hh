/*
 * PThreads.hh
 *
 * Copyright 2002, Emiliano Martin emilianomc@terra.es All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef _LOG4CPP_THREADING_PTHREADS_HH
#define _LOG4CPP_THREADING_PTHREADS_HH

#include <log4cpp/Portability.hh>
#include <stdio.h>
#include <pthread.h>
#include <string>
#include <assert.h>


namespace log4cpp {
    namespace threading {

        /** 
         * returns the thread ID
         **/
        std::string getThreadId();
        
        /**
         **/
        class Mutex {
            private:
            pthread_mutex_t mutex;

            public:
              Mutex() {
                ::pthread_mutex_init(&mutex, NULL);
            }

              void lock() {
                ::pthread_mutex_lock(&mutex);
            }

              void unlock() {
                ::pthread_mutex_unlock(&mutex);
            }

              ~Mutex() {
                ::pthread_mutex_destroy(&mutex);
            }

            private:
            Mutex(const Mutex& m);
            Mutex& operator=(const Mutex &m);
        };

        /**
         *	definition of ScopedLock;
         **/
        class ScopedLock {
            private:
            Mutex& _mutex;

            public:
              ScopedLock(Mutex& mutex) :
                _mutex(mutex) {
                _mutex.lock();
            }

              ~ScopedLock() {
                _mutex.unlock();
            }
        };

        /**
         * 
         **/
        template<typename T> class ThreadLocalDataHolder {
            private:            
            pthread_key_t _key;              

            public:
            typedef T data_type;

              ThreadLocalDataHolder() {
                ::pthread_key_create(&_key, freeHolder);         
            }

              static void freeHolder(void *p) {
                assert(p != NULL);
                delete reinterpret_cast<T *>(p);
             }

              ~ThreadLocalDataHolder() {
                T *data = get();
                if (data != NULL) { 
                    delete data;
                }
                ::pthread_key_delete(_key);
            }
            
              T* get() const {
                return reinterpret_cast<T *>(::pthread_getspecific(_key)); 
            }

              T* operator->() const { return get(); }
              T& operator*() const { return *get(); }

              T* release() {
                T* result = get();
                ::pthread_setspecific(_key, NULL); 

                return result;
            }

              void reset(T* p = NULL) {
                T *data = get();
                if (data != NULL) {
                    delete data;
                }
                ::pthread_setspecific(_key, p); 
            }
        };

    }
}
#endif
