#include "Log.hpp"

#include <iostream>
#include <sstream>

/**
 * Log.cpp
 * Implements Log.hpp.
 *
 * @author Sean Scott
 */
 
Log::Log(int _id, int _weight, std::string _time)
{
    setLog(_id, _weight, _time);
}

void Log::setLog(int _id, int _weight, std::string _time)
{
    id = _id;
    weight = _weight;
    time = _time;
}

std::string Log::toString()
{
    std::stringstream ss;
    ss << "ID: "       << id
       << ", Weight: " << weight
       << ", Time: "    << time;
       
    return ss.str();
}
