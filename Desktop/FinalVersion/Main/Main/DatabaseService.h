#pragma once

#include "sqlite3.h"
#include <string>
#include <vector>
#include <filesystem>
#include "Friend.h"
#include "Message.h"
#include "Nickname.h"
#include "User.h"

class DatabaseService {
public:
    static bool InitializeUserDB(const std::string& username, const std::string& password);
    static void CloseDB();

    static bool ExecuteSQL(const std::string& sql);
    static void InitializeSchema();
    static void SyncFriendsToDB(const std::vector<Friend>& friends);
    static void SaveMessageToDB(const Message& msg, const std::string& senderId, const std::string& receiverId);
    static void SaveNicknameToDB(const NicknameInfo& info);
    static std::vector<Message> LoadMessages(const std::string& myId, const std::string& friendId);
    static std::vector<MediaItem> LoadImagesForMessage(const std::string& messageId);
    static std::vector<MediaItem> LoadFilesForMessage(const std::string& messageId);
    static bool SaveUserInfo(const User& user);
    static void EnsureSenderExists(const std::string& senderId);

    static sqlite3* m_db; 
};