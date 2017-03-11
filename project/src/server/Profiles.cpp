#include "Profiles.hpp"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <vector>
#include <jsoncpp/json/json.h>

using namespace std;

/**
 * Profiles.cpp
 * Implements Profiles.hpp
 *
 * @author Sean Scott
 */

void Profiles::initListFromJsonFile(std::string jsonFilename)
{
    reset();

    Json::Reader reader;
    Json::Value root;
    std::ifstream json(jsonFilename.c_str(), std::ifstream::binary);
    bool parseSuccess = reader.parse(json, root, false);
    if (parseSuccess)
    {
        for (Json::ValueIterator i = root.begin(); i != root.end(); ++i)
        {
            const Json::Value& jsonUser = *i;

            int id = jsonUser["id"].asInt();
            string name = jsonUser["name"].asString();
            int height = jsonUser["height"].asInt();
            int age = jsonUser["age"].asInt();
            string gender = jsonUser["gender"].asString();
            vector<Log*> logs;

            const Json::Value& jsonLogs = jsonUser["logs"];
            for (Json::ValueIterator j = jsonLogs.begin(); j != jsonLogs.end(); ++j)
            {
                const Json::Value& jsonLog = *j;

                int logId = jsonLog["id"].asInt();
                int weight = jsonLog["weight"].asInt();
                string time = jsonLog["time"].asString();

                Log* log = new Log(id, weight, time);
                logs.push_back(log);
            }

            Profile* user = new Profile(id, name, height, age, gender, logs);

            cout << "USER: " << user->toString() << endl;

            add(user);
        }
    }
}

void Profiles::toJsonFile(std::string jsonFilename)
{
    Json::Value root;

    for (int i = 0; i < list.size(); i++)
    {
        Json::Value jsonUser;

        Profile* user = list.at(i);

        jsonUser["id"] = user->getId();
        jsonUser["name"] = user->getName();
        jsonUser["height"] = user->getHeight();
        jsonUser["age"] = user->getAge();
        jsonUser["gender"] = user->getGender();

        Json::Value jsonLogs;

        for (int j = 0; j < user->getLogs().size(); j++)
        {
            Json::Value jsonLog;

            Log* log = user->getLogs().at(j);

            jsonLog["id"] = log->getId();
            jsonLog["weight"] = log->getWeight();
            jsonLog["time"] = log->getTime();

            jsonLogs[j] = jsonLog;
        }

        jsonUser["logs"] = jsonLogs;

        root[i] = jsonUser;
    }

    Json::StyledStreamWriter ssw(" ");
    std::ofstream jsonOutFile(jsonFilename.c_str(), std::ofstream::binary);
    ssw.write(jsonOutFile, root);
    cout << "Exported to JSON" << endl;
}

bool Profiles::add(Profile* user)
{
    list.push_back(user);

    return true;
}

bool Profiles::remove(int id)
{
    for (int i = 0; i < list.size(); i++)
    {
        if (list.at(i)->getId() == id)
        {
            cout << "Removing " << id << endl;
            list.erase(list.begin()+i);
            return true;
        }
    }

    return false;
}
/*
Profile* Profiles::at(int index)
{
    return list.at(index);
}
*/
Profile* Profiles::get(int id)
{
    for (int i = 0; i < list.size(); i++)
    {
        if (list.at(i)->getId() == id)
        {
            return list.at(i);
        }
    }

    return new Profile();
}

std::vector<Profile*> Profiles::toVector()
{
    return list;
}

int Profiles::size()
{
  return list.size();
}

void Profiles::reset()
{
  list.clear();
}
