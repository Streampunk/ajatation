#pragma once

#define ENABLE_TRACE

#ifdef ENABLE_TRACE
#include <string>
#define TRACE_LOG(message) _outputTrace(message)
void _outputTrace(std::string message);
#else
#define TRACE_LOG(message) 0
#endif