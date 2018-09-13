//
//  config.hpp
//  
//
//  Created by Edhay on 09/09/18.
//

#ifndef config_hpp
#define config_hpp
#include <stdio.h>
#include <unordered_map>
#include <string>

#define COUT(arg) if (1 == GlobalConfig::verboseMode ) cout arg

const int INVALID = -1;
const int ERROR_VAL = -1;

using namespace std;

class Config
{
    public:
    
    static Config *getInstance()
    {
        if (!s_configInstance)
            s_configInstance = new Config;
        return s_configInstance;
    }
    
        int init(string configFileName)
    {
        return readConfigFile(configFileName);
    }
    
    int getConfigValue(string name);
    
    
    private:
    static Config *s_configInstance;
    Config()
    {
    }
    

    int readConfigFile(string configFileName);
    unordered_map   <string, int> nameValue;
    string configFileName;
    
};





#endif /* config_hpp */
