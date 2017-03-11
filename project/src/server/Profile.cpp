#include "Profile.hpp"

#include <iostream>
#include <sstream>

/**
 * Profile.cpp
 * Implements Profile.hpp.
 *
 * @author Sean Scott
 */
 
Profile::Profile(int _id, std::string _name, int _height, int _age, std::string _gender, std::vector<Log*> _logs)
{
    setProfile(_id, _name, _height, _age, _gender, _logs);
}

void Profile::setProfile(int _id, std::string _name, int _height, int _age, std::string _gender, std::vector<Log*> _logs)
{
    id = _id;
    name = _name;
    height = _height;
    age = _age;
    gender = _gender;
    logs = _logs;
}

bool Profile::removeLog(int id)
{
  for (int i = 0; i < logs.size(); i++)
  {
    if (logs.at(i)->getId() == id)
    {
      logs.erase(logs.begin() + i);
      return true;
    }
  }

  return false;
}

std::string Profile::toString()
{
    std::stringstream ss;
    ss << "ID: "       << id
       << ", Name: "   << name
       << ", Height: " << height
       << ", Age: "    << age
       << ", Gender: " << gender;
       
    return ss.str();
}
