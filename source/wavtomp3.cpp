#include <iostream>
#include <string>
#include <csignal>
#include <vector>
#include <map>
//#include "config.hpp"
#include "globalconfig.hpp"

//extern "C" {
#include <stdio.h>
#include <string.h>
#include <cstdio>

#ifdef _WIN32
// Microsoft Windows Specific headers
#include <windows.h>
#include <direct.h>
#define HAVE_STRUCT_TIMESPEC
#else
#include "lame.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

extern "C" {
#include "lame.h"
#define PTW32_STATIC_LIB
#include "pthread.h"
}

#include "threadexception.hpp"
#include "threadwrapper.hpp"



using namespace std;

enum BlockSize
{
    WAV_BLOCK_SIZE = 8192,
    MP3_BLOCK_SIZE = 8192
};

namespace Utility
{
    //Getting the number of CPUs in Linux
    int getNumberOfCPU()
    {
        int numCPU = INVALID;

    #ifdef __linux__
        numCPU = (int)sysconf(_SC_NPROCESSORS_ONLN);

        if (numCPU < 0)
        {
            numCPU = 1;
        }
    #elif _WIN32
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        numCPU = sysinfo.dwNumberOfProcessors;
    #endif

        return (numCPU);
    }

    size_t ReadDirectory(string dirPath, multimap<long long, string> &hold)
    {
        
        #ifdef __linux__
        DIR* dirFile = opendir( dirPath.c_str());
        if (NULL == dirFile )
        {
            cout<<"\nError: could not open the directory " << dirPath.c_str() << " to read.\n";
            return ERROR_VAL;
        }
        struct dirent* dirStruct = NULL;
        bool bIgnoreHidden = true;
        while (( dirStruct = readdir( dirFile )) != NULL )
        {
            if ( !strncmp( dirStruct->d_name, ".", 2  )) continue;
            if ( !strncmp( dirStruct->d_name, "..", 3 )) continue;
            
            // in linux hidden files all start with '.'
            if ( bIgnoreHidden && ( dirStruct->d_name[0] == '.' )) continue;
            
            string wavFileName;
            //Check if the filename contains with .wav of .WAV. (Ignore all other files)
            if ( strstr( dirStruct->d_name, ".wav" ) || strstr( dirStruct->d_name, ".WAV" ))
            {
                
                //Creating the full path
                wavFileName = dirPath + "/" + dirStruct->d_name;
                
                //get the size of this file
                struct stat st;
                size_t filesize = 0;
                int ret = stat(wavFileName.c_str(), &st);
                if (ret != 0 )
                {
                    //cout<<"\nError: Getting filesize failed. setting to size to 1";
                    filesize = 1;
                }
            	else
            	{
                    filesize = st.st_size;
            	}
                //Insert into the multimap. This list be intrinsically sorted
                hold.insert(pair<long long, string> (filesize, wavFileName));
            
            }
        }
        closedir( dirFile );

        #elif _WIN32
        WIN32_FIND_DATAA data;
        string searchPath = dirPath + "\\*";
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &data);      // DIRECTORY

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                //check if is a wav file
                if (strstr(data.cFileName, ".wav") || strstr(data.cFileName, ".WAV"))
                {
                    string wavFileName = dirPath + "\\" + data.cFileName; 
                    //cout << wavFileName << endl;
					COUT( << wavFileName << endl);

                    size_t filesize = 1;
                    
                    filesize = data.nFileSizeHigh;
                    filesize <<= sizeof(data.nFileSizeHigh) * 8;
                    filesize |= data.nFileSizeLow;
                    
                    //cout << "The filesize : " << filesize << endl;
					COUT( << "The filesize : " << filesize << endl);

                    //Insert into the multimap. This list be intrinsically sorted
                    hold.insert(pair<long long, string>(filesize, wavFileName));
                }
            } while (FindNextFileA(hFind, &data));
            FindClose(hFind);
        }
        else
        {
            cout << "Filehandle got is NULL \n";
            return ERROR_VAL;
        }
        #endif

        return(hold.size());
    }

    int CheckIfDir(string path)
    {
        #ifdef __linux__
        struct stat s;
        if( stat(path.c_str(),&s) == 0 )
        {
            if( s.st_mode & S_IFDIR )
            {
                return 0;
            }
        }
        #elif _WIN32 
        if( FILE_ATTRIBUTE_DIRECTORY == GetFileAttributesA(path.c_str()))
        {
        	return 0;
        }
        #endif
        
        return ERROR_VAL;
        
    }

}


const int GlobalConfig::DELETE_ORIGINAL_FILES;
const int GlobalConfig::DEFAULT_THREAD_MULTIPLE;
const int GlobalConfig::DEFAULT_BIT_SAMPLING_RATE;
const int GlobalConfig::DEFAULT_VERBOSE_MODE;
int GlobalConfig::deleteOriginalFiles = DELETE_ORIGINAL_FILES;
int GlobalConfig::nrOfThreadsPerProcesser = DEFAULT_THREAD_MULTIPLE;
int GlobalConfig::bitSamplingRate= DEFAULT_BIT_SAMPLING_RATE;
int GlobalConfig::verboseMode = DEFAULT_VERBOSE_MODE;



//Declaration of thread function
typedef void* (*RunnerFunc)(void*);

//Declration of a major implementation of thread function
int EncodeWrapper(vector<string> wavFiles);


//The Function that actually converts from .wav to mp3
int ConvertWavToMp3(FILE *wav, FILE *mp3, lame_t lame)
{
    short int wav_buffer[WAV_BLOCK_SIZE*2] = {0x0};
    unsigned char mp3_buffer[MP3_BLOCK_SIZE] = {0x0};
    size_t read = -1;
    int write = -1;
    
    do
    {
        read = fread(wav_buffer, 2 * sizeof(short int), WAV_BLOCK_SIZE, wav);
        if(ferror(wav))
        {
            //cout<<"\nRead wav file failed\n";
            COUT(<<"\nRead wav file failed\n");
            return ERROR_VAL;
        }
        if (0 == read)
        {   //already read till EOF
            write = lame_encode_flush(lame, mp3_buffer, MP3_BLOCK_SIZE);
        }
        else
        {
            write = lame_encode_buffer_interleaved(lame, wav_buffer, (int)read, mp3_buffer, MP3_BLOCK_SIZE);
        }
        fwrite(mp3_buffer, write, 1, mp3);
        if(ferror(wav))
        {
            //cout<<"\nWrite mp3 file failed\n";
            COUT(<<"\nWrite mp3 file failed\n");
            return ERROR_VAL;
        }
    } while (read != 0);
    
    return 0;
}

void* threadRunner(void* args)
{
    
    vector<string> *vectorArg = static_cast < vector<string>* > (args);
    vector<string> wavFiles = *vectorArg;
    size_t size = wavFiles.size();
    if( 0  == size)
    {
        //Nothing to do
        return NULL;
    }
    //cout<<"No. of .wav files to be processed by this thread : "<< size;
    COUT(<<"No. of .wav files to be processed by this thread : "<< size);

    EncodeWrapper(wavFiles);
    return (NULL);
}




int EncodeWrapper(vector<string> wavFiles)
{
    //Initialise the lame
    lame_t lame = lame_init();
    if(NULL == lame)
    {
        //cout<<"Error: lame_init() failed. Perhaps out of memory";
        COUT(<<"Error: lame_init() failed. Perhaps out of memory");
        return ERROR_VAL;
    }
    lame_set_in_samplerate(lame, 44100);
    lame_set_VBR(lame, vbr_default);
    int rval = lame_init_params(lame);
    if( -1 == rval )
    {
        //cout<<"Error: lame_init_params() failed.";
        COUT(<<"Error: lame_init_params() failed.");
        return ERROR_VAL;
    }
    
    long count = 0;
    for(auto filename : wavFiles)
    { 
	    //cout<<"\n filename : "<<filename<<endl;
        FILE *mp3 = NULL;
        FILE *wav = fopen(filename.c_str(), "rb");
        if(NULL == wav)
        {
            //cout<<"\n could not open "<<filename<<", ignoring this file\n";
            COUT(<<"\n could not open "<<filename<<", ignoring this file\n");
            //Skip this file.
            continue;
        }
        string mp3FileName(filename);
        
        //To get the mp3 taget filename,  change the file extension to .wav to .mp3
        mp3FileName.replace(mp3FileName.length() - 3, 3, "mp3");
        COUT(<<"\nTrying to write to "<<mp3FileName.c_str()<<" \n");
        mp3 = fopen(mp3FileName.c_str(), "wb");
        if(NULL == mp3 )
        {
            //cout<<"\n could not open file.mp3 to write, ignoring this file\n";
            COUT(<<"\n could not open file.mp3 to write, ignoring this file\n");
            fclose(wav);
            continue;
        }
        //cout<<"\nCalling Convert...\n";
        int ret = ConvertWavToMp3(wav, mp3, lame);
        count++;
       
        fclose(wav);
        fclose(mp3);

        if( 0 == ret && 1 == GlobalConfig::deleteOriginalFiles )
        {
            if( 0 != remove( filename.c_str() ) )
            {
                //cout<<"Error : Removing file "<< filename <<" Failed";
                COUT(<<"Error : Removing file "<< filename <<" Failed");
            }
        }
       
    }
    
    lame_close(lame);
    //cout<<"\nTotal Files Processed = "<<count<<"\n";
    COUT(<<"\nTotal Files Processed = "<<count<<"\n");
    return 0;
}




int main(int argc, char* argv[])
{
    
    
    if( argc != 2)
    {
        cout<<"\nusage : " << argv[0] << " <Path of directory with .wav files> \n\n";
        return (1);
    }
    
    string dirPath( argv[1] );

    if( 0 != Utility::CheckIfDir(dirPath))
    {
        cerr<< dirPath << " is not a valid directory. Exiting ...\n\n";
        return (1);
    }

    //cout<<"Working on dir : " << dirPath << "\n";
    COUT(<<"Working on dir : " << dirPath << "\n");
    
    GlobalConfig::InitConfig();

    multimap<long long, string> hold;
    
	if( ERROR_VAL == (int)Utility::ReadDirectory(dirPath, hold))
    {
        cerr<<"\nError: Reading directory failed. Exiting";
        return (1);
    }
    
    size_t nrOfWavFiles = hold.size();
    //cout<<"Total number of .wav files found : " <<nrOfWavFiles<<endl;
    COUT(<<"Total number of .wav files found : " <<nrOfWavFiles<<endl);

	
    if(0 == nrOfWavFiles)
    {
        cerr<<"No .wav files found in "<<dirPath<<" so exiting ...\n";
        return 0;
    }
    

    int nrOfCPUs = Utility::getNumberOfCPU();

	if ( nrOfCPUs < 0 )
	{
		//cout << "Total number of CPUs cores: " << nrOfCPUs <<" So setting it to 1"<< endl;
		COUT( << "Total number of CPUs cores: " << nrOfCPUs <<" So setting it to 1"<< endl);
		nrOfCPUs = 1;
	}

    //cout<<"Total number of CPUs cores: " <<nrOfCPUs<<endl;
    COUT(<<"Total number of CPUs cores: " <<nrOfCPUs<<endl);

    //Choosing 4 threads per processor
    size_t nrOfThreads = nrOfCPUs * GlobalConfig::nrOfThreadsPerProcesser;
    //cout<<"Total number Threads is set to : " <<nrOfThreads<<" Thread Multiple is :" <<GlobalConfig::nrOfThreadsPerProcesser<<endl;
    COUT(<<"Total number Threads is set to : " <<nrOfThreads<<" Thread Multiple is :" <<GlobalConfig::nrOfThreadsPerProcesser<<endl);

	if (nrOfThreads > nrOfWavFiles)
	{
		nrOfThreads = nrOfWavFiles;
		//cout << "Total number Threads is reset to : " << nrOfThreads;
		COUT(<< "Total number Threads is reset to : " << nrOfThreads);
	}

    //To store the subset of the wav files to send to the induvidual threads.
    vector <string> *sub = new vector <string>[nrOfThreads];
    
    //Distribute the files to sub. Each thread will work on 1 sub
    int index = 0;
    for(multimap<long long,string>::iterator x = hold.begin(); x != hold.end(); x++ )
    {
        sub[index].push_back(x->second);
        index = (index + 1) % nrOfThreads;
        //cout<<"\n "<<x->first<<" "<<x->second;
    }
    
    cout<<"\nDistribution\n";
    for(size_t i=0; i<nrOfThreads; i++)
    {
        for ( auto x : sub[i])
        {
            //cout<<"\nSUB : "<<i<< " "<<x;
            COUT(<<"\nSUB : "<<i<< " "<<x);
        }
        //cout<<"\nCount : "<<sub[i].size();
        COUT(<<"\nCount : "<<sub[i].size());
    }
    
    
    vector<Thread> converters;
    
    for(size_t i=0; i<nrOfThreads; i++)
    {
        try
        {
            Thread worker(threadRunner, &sub[i]);
            converters.push_back(worker);
        }
        catch( ThreadException t)
        {
            cerr << "ThreadException Exception Caught, Message : " << t.getMessage() << " Status : "<< t.getStatus() << " \n";
        }
    }
    
    //Wait for all threads before exiting the main
    Thread::threadJoin(converters);
   
    delete [] sub;
    return 0;
}
