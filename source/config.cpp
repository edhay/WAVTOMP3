//
//  config.cpp
//  
//
//  Created by Edhay on 09/09/18.
//

#include "config.hpp"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <iostream>


Config* Config::s_configInstance = NULL;
using namespace std;


int Config::readConfigFile(string configFileName)
{
    std::ifstream cFile (configFileName);
    if (cFile.is_open())
    {
        std::string line;
        while(getline(cFile, line))
        {
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace),
                       line.end());
            if(line[0] == '#' || line.empty())
                continue;
            int delimiterPos = line.find("=");
            string name = line.substr(0, delimiterPos);
            string value = line.substr(delimiterPos + 1);
            //cout << name << " " << value << '\n';
            int val = -1;
            try
            {
                val = std::stoi(value);
            }
            catch (...)
            {
                //cout<< "Could not covert to int. Config Reading Failed\n";
                return -1;
            }
            nameValue.insert(std::pair<string, int>(name, val));
        }
        
        return 0;
        
    }
    else
    {
        //cout << "Couldn't open config file for reading.\n";
        return -1;
    }
}


int Config::getConfigValue(string name)
{
    unordered_map<string,int>::const_iterator entry = nameValue.find(name);
    
    if ( entry == nameValue.end() )
    {
        cout << "Warning : not found config : "<< name <<"\n";
        return -1;
    }
    else
    {
        //cout <<"From Config "<<"settings.cfg  "<< entry->first << " : " << entry->second<<endl;
        return entry->second;
    }
    
}
