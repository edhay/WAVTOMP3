//
//  config.hpp
//  
//
//  Created by Edhay on 09/09/18.
//

#ifndef globalconfig_hpp
#define globalconfig_hpp


#include "config.hpp"

class GlobalConfig
{
private:
	static const int DELETE_ORIGINAL_FILES = 1;
	static const int DEFAULT_THREAD_MULTIPLE = 4;
	static const int DEFAULT_BIT_SAMPLING_RATE = 44100;
	static const int DEFAULT_VERBOSE_MODE = 0;
public:
	static int deleteOriginalFiles;
	static int nrOfThreadsPerProcesser;
	static int bitSamplingRate;
	static int verboseMode;

	static void InitConfig()
	{
		Config* configuration = Config::getInstance();
		if (NULL == configuration || -1 == configuration->init("config/settings.cfg"))
		{
			cout << "Could not read config file. Setting to default values\n";
		}
		else
		{
			int tempvalue = configuration->getConfigValue("DeleteOriginalFiles");
			if (tempvalue != INVALID && (tempvalue == 0 || tempvalue == 1))
			{
				deleteOriginalFiles = tempvalue;
			}
			tempvalue = configuration->getConfigValue("NrOfThreadsPerProcesser");
			if (tempvalue != INVALID && (tempvalue >= 1 && tempvalue <= 16))
			{
				nrOfThreadsPerProcesser = tempvalue;
			}
			tempvalue = configuration->getConfigValue("BitSamplingRate");
			if (tempvalue != INVALID)
			{
				bitSamplingRate = tempvalue;
			}
			tempvalue = configuration->getConfigValue("VerboseMode");
			if (tempvalue != INVALID && (tempvalue == 0 || tempvalue == 1))
			{
				verboseMode = tempvalue;
			}
		}

		//cout << "\nConfiguration Values: \n";
		//cout << "DeleteOriginalFiles = " << deleteOriginalFiles << "\n";
		//cout << "NrOfThreadsPerProcesser = " << nrOfThreadsPerProcesser << "\n";
		//cout << "BitSamplingRate = " << bitSamplingRate << "\n";

		COUT( << "\nConfiguration Values: \n");
		COUT( << "DeleteOriginalFiles = " << deleteOriginalFiles << "\n");
		COUT( << "NrOfThreadsPerProcesser = " << nrOfThreadsPerProcesser << "\n");
		COUT( << "BitSamplingRate = " << bitSamplingRate << "\n");
	}

};

#endif