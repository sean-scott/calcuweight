#include <iostream>
#include <sstream>
#include <string>

/**
 * Log.hpp
 * Defines properties of a weight log.
 *
 * @author Sean Scott
 */
 
// Header guard
#ifndef LOG_H
#define LOG_H

class Log
{
private:
    int id;
    int weight;
    std::string time;
public:
    Log() {}
    
    Log(int _id, int _weight, std::string _time);
    
    void setLog(int _id, int _weight, std::string _time);
    
    int getId() { return id; }
    int getWeight() { return weight; }
    std::string getTime() { return time; }
    
    std::string toString();
};

#endif
