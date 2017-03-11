#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Log.hpp"

/**
 * Profile.hpp
 * Defines properties of a user profile.
 *
 * @author Sean Scott
 */
 
// Header guard
#ifndef PROFILE_H
#define PROFILE_H

class Profile
{
private:
    int id;
    std::string name;
    int height;
    int age;
    std::string gender;
    std::vector<Log*> logs;
public:
    Profile() {}

    Profile(int _id, std::string _name, int _height, int _age, std::string _gender, std::vector<Log*> _logs);

    void setProfile(int _id, std::string _name, int _height, int _age, std::string _gender, std::vector<Log*> _logs);

    int getId() { return id; }
    std::string getName() { return name; }
    int getHeight() { return height; }
    int getAge() { return age; }
    std::string getGender() { return gender; }
    std::vector<Log*> getLogs() { return logs; }

    bool removeLog(int id);
    std::string toString();
};

#endif
