//
//  ThreadWrapper.hpp
//  
//
//  Created by Edhay on 27/08/18.
//

#ifndef ThreadWrapper_hpp
#define ThreadWrapper_hpp

#include <stdio.h>
#include <vector>
#include "threadexception.hpp"

using namespace std;

typedef void* (*RunnerFunc)(void*);
//A Simple thread wrapper for pthread
class Thread
{
    public:
    Thread(RunnerFunc r, void * args):runner(r)
    {
        int ret = 0 ;
        
        ret = pthread_attr_init(&attr);
        if ( ret != 0)
        {
            throw ThreadException("Error in pthread_attr_init", ret );
        }
        
        ret = pthread_create(&threadID, &attr, runner, args);
        if ( ret != 0)
        {
            throw ThreadException("Error in  pthread_create.", ret );
        }
        
		#ifdef __linux__
		cout<<"Thread "<<threadID<<" Started Successfully \n";
        #elif _WIN32
		cout << "Thread  Started Successfully \n";
        #endif
        
    }
    
    
    static void threadJoin(vector<Thread> &threads)
    {
        for (size_t i = 0; i < threads.size(); i++)
        {
            pthread_join(threads[i].threadID, NULL);
            //cout<<"\n ThreadID : "<<threads[i].threadID<<" completed";
			cout << "\n Thread : " << i << " completed";
        }
        cout<<"\n";
    }
    
    private:
    pthread_t threadID;
    pthread_attr_t attr;
    RunnerFunc runner;
};

#endif /* ThreadWrapper_hpp */
