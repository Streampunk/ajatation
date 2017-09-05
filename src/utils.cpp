#include "utils.h"

#ifdef ENABLE_TRACE
#include <iostream>

using namespace std;

void _outputTrace(std::string message)
{
    cout << message << endl;
}

#endif