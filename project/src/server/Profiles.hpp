#include "Profile.hpp"

#include <iostream>
#include <string>
#include <map>
#include <vector>

/**
 * Profiles.hpp
 * Defines properties of a list of profiles
 *
 * @author Sean Scott
 */

 // Header guard
 #ifndef PROFILES_H
 #define PROFILES_H

class Profiles
{
private:
    std::vector<Profile*> list; // the list that holds all Profile objects
protected:
    std::map<std::string, Profile> users;
public:
    Profiles() {}

    void reset();

    void initListFromJsonFile(std::string jsonFilename);
    void toJsonFile(std::string jsonFilename);

    bool add(Profile* aUser);

    bool remove(int id);

//    Profile* at(int index);
    Profile* get(int id);

    std::vector<Profile*> toVector();

    int size();
 };

 #endif
