#include "pch.h" 
#include "DatabaseService.h"
#include <iostream>

sqlite3* DatabaseService::m_db = nullptr;

bool DatabaseService::OpenDB(const std::string& dbPath) {
    int rc = sqlite3_open(dbPath.c_str(), &m_db);
    return (rc == SQLITE_OK);
}

void DatabaseService::CloseDB() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool DatabaseService::ExecuteSQL(const std::string& sql) {
    if (!m_db) return false;
    char* errorMessage = nullptr;
    int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errorMessage);

    if (rc != SQLITE_OK) {
        sqlite3_free(errorMessage);
        return false;
    }
    return true;
}


void DatabaseService::InitializeSchema() {
    ExecuteSQL("PRAGMA foreign_keys = ON;");

    // 1. Bảng Users: Thêm DEFAULT 'avatar/default.jpg'
    ExecuteSQL("CREATE TABLE IF NOT EXISTS Users ("
        "userId INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE, "
        "fullName TEXT, "
        "password TEXT, "
        "avatar TEXT DEFAULT 'avatar/default.jpg');");

    // 2. Bảng Friends: Thêm DEFAULT 'avatar/default.jpg'
    ExecuteSQL("CREATE TABLE IF NOT EXISTS Friends ("
        "friendId INTEGER PRIMARY KEY AUTOINCREMENT, "
        "userId INTEGER, "
        "fullName TEXT, "
        "avatar TEXT DEFAULT 'avatar/default.jpg', "
        "isOnline INTEGER DEFAULT 0, "
        "FOREIGN KEY(userId) REFERENCES Users(userId));");

    // 3. Bảng Messages
    ExecuteSQL("CREATE TABLE IF NOT EXISTS Messages ("
        "messageId INTEGER PRIMARY KEY AUTOINCREMENT, "
        "senderId INTEGER, "
        "content TEXT, "
        "createdAt TEXT, "
        "FOREIGN KEY(senderId) REFERENCES Users(userId));");

    // 4. Bảng Images
    ExecuteSQL("CREATE TABLE IF NOT EXISTS Images ("
        "imageId INTEGER PRIMARY KEY AUTOINCREMENT, "
        "messageId INTEGER, "
        "url TEXT, "
        "FOREIGN KEY(messageId) REFERENCES Messages(messageId) ON DELETE CASCADE);");

    // 5. Bảng Files
    ExecuteSQL("CREATE TABLE IF NOT EXISTS Files ("
        "fileId INTEGER PRIMARY KEY AUTOINCREMENT, "
        "messageId INTEGER, "
        "url TEXT, "
        "fileName TEXT, "
        "FOREIGN KEY(messageId) REFERENCES Messages(messageId) ON DELETE CASCADE);");
}