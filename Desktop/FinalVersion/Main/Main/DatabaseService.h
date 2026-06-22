#pragma once
#include "sqlite3.h"
#include <string>
#include "Friend.h"
#include <vector>
#include "Message.h"
#include "Nickname.h"

class DatabaseService {
public:
    static bool OpenDB(const std::string& dbPath);

    static void CloseDB();

    static bool ExecuteSQL(const std::string& sql); 

    static void InitializeSchema();

    static sqlite3* m_db;
    static void SyncFriendsToDB(const std::vector<Friend>& friends);
    static void SaveMessageToDB(const Message& msg, const std::string& senderId);
    static void SaveNicknameToDB(const NicknameInfo& info);
};