#include <jsonrpccpp/server.h>
#include <jsonrpccpp/server/connectors/httpserver.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "Profile.hpp"
#include "Profiles.hpp"
#include "Log.hpp"
#include "serverstub.h"

using namespace jsonrpc;
using namespace std;

/**
 * Server.cpp
 * Server-side processing
 *
 * @author Sean Scott
 */
class Server : public serverstub
{
private:
    int portNum;
    Profiles users;
public:
    Server(AbstractServerConnector &connector, int port);

    virtual void notifyServer();
    virtual bool addUser(const Json::Value& user);
    virtual bool removeUser(int id);
    virtual Json::Value getUser(int index);
    virtual int getUserCount();
    virtual bool removeLog(int userId, int logId);
};

Server::Server(AbstractServerConnector &connector, int port) : serverstub(connector)
{
    users.initListFromJsonFile("server.json");
    portNum = port;
    cout << "Server up and listening on port " << port << endl;
}

void Server::notifyServer()
{
    cout << "Server notified" << endl;
}

bool Server::addUser(const Json::Value& user)
{
    cout << "Adding..." << endl;

    int id = user["id"].asInt();

    if (id == -1)
    {
        int maxId = -1;
        for (Profile* aUser : users.toVector())
        {
            if (aUser->getId() > maxId)
            {
                maxId = aUser->getId();
            }
        }

        id = maxId + 1;
    }
    else
    {
        users.remove(id);
    }

    string name = user["name"].asString();
    int height = user["height"].asInt();
    int age = user["age"].asInt();
    string gender = user["gender"].asString();

    vector<Log*> logs;

    Json::Value logsRoot = user["logs"];

    for (int j = 0; j < logsRoot.size(); j++)
    {
        Json::Value jsonLog = logsRoot[j];

        int logId = jsonLog["id"].asInt();
        int weight = jsonLog["weight"].asInt();
        string time = jsonLog["time"].asString();

        Log* log = new Log(logId, weight, time);
        logs.push_back(log);
    }

    Profile* usr = new Profile(id, name, height, age, gender, logs);

    users.add(usr);
    users.toJsonFile("server.json");

    return true;
}

bool Server::removeUser(int id)
{
    users.remove(id);
    users.toJsonFile("server.json");
    return true;
}

Json::Value Server::getUser(int id)
{
    // refresh in case of new weight log
    users.initListFromJsonFile("server.json");

    Json::Value jsonUser;

    Profile* user = users.get(id);

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

    return jsonUser;
}

int Server::getUserCount()
{
    // refresh in case of new weight log
    users.initListFromJsonFile("server.json");

    cout << "count: " << users.size() << endl;

    return users.size();
}

bool Server::removeLog(int userId, int logId)
{
    bool success = users.get(userId)->removeLog(logId);
    users.toJsonFile("server.json");
    return success;
}

int main(int argc, char* argv[])
{
    int port = 7070;
    if (argc > 1)
    {
        port = atoi(argv[1]);
    }

    HttpServer httpserver(port);
    Server cs(httpserver, port);
    cs.StartListening();
    int c = getchar();
    cs.StopListening();
    return 0;
}
