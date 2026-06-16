#pragma once
#include "sqlite3.h"
#include <string>

class DatabaseService {
public:
    static bool OpenDB(const std::string& dbPath);

    static void CloseDB();

    static bool ExecuteSQL(const std::string& sql);

    static void InitializeSchema();

    static sqlite3* m_db;
};