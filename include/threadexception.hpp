//
//  threadexception.hpp
//  BasicXCode
//
//  Created by Edhay on 01/09/18.
//  Copyright © 2018 Edhay. All rights reserved.
//

#ifndef threadexception_hpp
#define threadexception_hpp
#include <string>
using namespace std;

class ThreadException
{
public:
    ThreadException(string msg, int s) : message(msg), status(s)
    {
    }
    
    string getMessage()
    {
        return message;
    }
    
    int getStatus()
    {
        return status;
    }
    
    
private:
    string message;
    int status;
    
    
};

#endif /* threadexception_hpp */
