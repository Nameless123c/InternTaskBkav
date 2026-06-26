#include "pch.h" 
#include "DatabaseService.h"
#include <iostream>
#include "Friend.h"
#include <vector>
#include "Main.h"
#include "Message.h"
#include "Nickname.h"
#include <windows.h>


sqlite3* DatabaseService::m_db = nullptr;

bool DatabaseService::InitializeUserDB(const std::string& username, const std::string& password) {
    // 1. Đóng DB cũ nếu có
    CloseDB();

    // 2. Tạo cấu trúc thư mục Data/Username/
    namespace fs = std::filesystem;
    std::string folderPath = "Data/" + username;

    // Tự động tạo thư mục nếu chưa tồn tại
    if (!fs::exists(folderPath)) {
        fs::create_directories(folderPath);
    }

    std::string dbPath = folderPath + "/chat.db";

    // 3. Mở file DB
    if (sqlite3_open(dbPath.c_str(), &m_db) != SQLITE_OK) {
        return false;
    }

    // 4. Bắt buộc mã hóa (Dùng mật khẩu user làm KEY)
    // Lưu ý: Nếu là SQLCipher, hàm này sẽ mã hóa file nếu nó là file mới
    sqlite3_key(m_db, password.c_str(), (int)password.length());

    // 5. Kiểm tra tính hợp lệ bằng cách thực hiện 1 truy vấn đơn giản
    // Nếu sai mật khẩu, sqlite3_exec sẽ trả về lỗi
    if (sqlite3_exec(m_db, "SELECT count(*) FROM sqlite_master;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        CloseDB();
        return false;
    }

    // 6. Khởi tạo schema cho DB của User này
    InitializeSchema();
    return true;
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

    // 1. Bảng Users (ID lấy từ Server)
    ExecuteSQL("CREATE TABLE IF NOT EXISTS Users ("
        "userId TEXT PRIMARY KEY, " 
        "username TEXT UNIQUE, "
        "fullName TEXT, "
        "password TEXT, "
        "avatar TEXT DEFAULT 'avatar/default.jpg');");

    // 2. Bảng Friends (ID lấy từ Server)
    ExecuteSQL("CREATE TABLE IF NOT EXISTS Friends ("
        "friendId TEXT PRIMARY KEY, " 
        "userId TEXT, "              
        "fullName TEXT, "
        "avatar TEXT DEFAULT 'avatar/default.jpg', "
        "isOnline INTEGER DEFAULT 0, "
        "FOREIGN KEY(userId) REFERENCES Users(userId));");
        
    // 3. Bảng FriendSettings (Lưu biệt danh)
    ExecuteSQL("CREATE TABLE IF NOT EXISTS FriendSettings ("
        "id TEXT PRIMARY KEY, "
        "ownerId TEXT, "
        "friendId TEXT, "
        "nickname TEXT, "
        "FOREIGN KEY(ownerId) REFERENCES Users(userId) ON DELETE CASCADE, "
        "FOREIGN KEY(friendId) REFERENCES Friends(friendId) ON DELETE CASCADE);");

    // 4. Bảng Messages (Đã sửa khóa ngoại trỏ về bảng Users)
    ExecuteSQL("CREATE TABLE IF NOT EXISTS Messages ("
        "messageId TEXT PRIMARY KEY, "
        "senderId TEXT, "
        "receiverId TEXT, "
        "content TEXT, "
        "createdAt TEXT, "
        "messageType INTEGER, "
        "isSend INTEGER, "
        "FOREIGN KEY(senderId) REFERENCES Users(userId));");

    // 5. Bảng Images
    ExecuteSQL("CREATE TABLE IF NOT EXISTS Images ("
        "imageId TEXT PRIMARY KEY, "
        "messageId TEXT, "
        "url TEXT, "
        "FOREIGN KEY(messageId) REFERENCES Messages(messageId) ON DELETE CASCADE);");

    // 6. Bảng Files
    ExecuteSQL("CREATE TABLE IF NOT EXISTS Files ("
        "fileId TEXT PRIMARY KEY, "
        "messageId TEXT, "
        "url TEXT, "
        "fileName TEXT, "
        "FOREIGN KEY(messageId) REFERENCES Messages(messageId) ON DELETE CASCADE);");
}

void DatabaseService::SyncFriendsToDB(const std::vector<Friend>& friends) {
    // Dùng transaction để tăng tốc độ ghi (nếu không, mỗi lần INSERT sẽ mất thời gian tạo index)
    ExecuteSQL("BEGIN TRANSACTION;");

    std::string sql = "INSERT OR REPLACE INTO Friends (friendId, userId, fullName, avatar, isOnline) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(DatabaseService::m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        for (const auto& f : friends) {
            sqlite3_bind_text(stmt, 1, f.friendId.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, theApp.m_userData.userId.c_str(), -1, SQLITE_TRANSIENT); // userId của chủ sở hữu
            sqlite3_bind_text(stmt, 3, f.fullName.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, f.avatar.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 5, f.isOnline);

            sqlite3_step(stmt);
            sqlite3_reset(stmt); // Reset để tái sử dụng câu lệnh đã prepare
        }
        sqlite3_finalize(stmt);
    }
    ExecuteSQL("COMMIT;");
}


void DatabaseService::SaveMessageToDB(const Message& msg, const std::string& senderId, const std::string& receiverId) {
    // 1. Thêm receiverId vào câu lệnh SQL
    std::string sqlMsg = "INSERT OR REPLACE INTO Messages "
        "(messageId, senderId, receiverId, content, createdAt, messageType, isSend) "
        "VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmtMsg;

    if (sqlite3_prepare_v2(m_db, sqlMsg.c_str(), -1, &stmtMsg, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmtMsg, 1, msg.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmtMsg, 2, senderId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmtMsg, 3, receiverId.c_str(), -1, SQLITE_TRANSIENT); // Bind receiverId
        sqlite3_bind_text(stmtMsg, 4, msg.content.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmtMsg, 5, msg.createdAt.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmtMsg, 6, msg.messageType);
        sqlite3_bind_int(stmtMsg, 7, msg.isSend); // Lưu ý: tăng index bind lên 7

        sqlite3_step(stmtMsg);
        sqlite3_finalize(stmtMsg);
    }

    // 3. Lưu danh sách ảnh vào bảng Images
    if (!msg.images.empty()) {
        std::string sqlImg = "INSERT OR REPLACE INTO Images (imageId, messageId, url) VALUES (?, ?, ?);";
        sqlite3_stmt* stmtImg;
        if (sqlite3_prepare_v2(m_db, sqlImg.c_str(), -1, &stmtImg, nullptr) == SQLITE_OK) {
            for (const auto& img : msg.images) {
                sqlite3_bind_text(stmtImg, 1, img.id.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmtImg, 2, msg.id.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmtImg, 3, img.url.c_str(), -1, SQLITE_TRANSIENT);

                sqlite3_step(stmtImg);
                sqlite3_reset(stmtImg); // Reset statement để bind cho ảnh tiếp theo
            }
            sqlite3_finalize(stmtImg);
        }
    }

    // 4. Lưu danh sách tệp đính kèm vào bảng Files
    if (!msg.files.empty()) {
        std::string sqlFile = "INSERT OR REPLACE INTO Files (fileId, messageId, url, fileName) VALUES (?, ?, ?, ?);";
        sqlite3_stmt* stmtFile;
        if (sqlite3_prepare_v2(m_db, sqlFile.c_str(), -1, &stmtFile, nullptr) == SQLITE_OK) {
            for (const auto& file : msg.files) {
                sqlite3_bind_text(stmtFile, 1, file.id.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmtFile, 2, msg.id.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmtFile, 3, file.url.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmtFile, 4, file.fileName.c_str(), -1, SQLITE_TRANSIENT);

                sqlite3_step(stmtFile);
                sqlite3_reset(stmtFile); // Reset statement để bind cho file tiếp theo
            }
            sqlite3_finalize(stmtFile);
        }
    }

}

void DatabaseService::SaveNicknameToDB(const NicknameInfo& info) {
    ExecuteSQL("BEGIN TRANSACTION;");

    // Câu lệnh SQL: id là khóa chính, nếu đã tồn tại thì ghi đè (replace)
    std::string sql = "INSERT OR REPLACE INTO FriendSettings (id, ownerId, friendId, nickname) VALUES (?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    // Chuẩn bị câu lệnh
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        // Bind dữ liệu vào các dấu hỏi chấm (?)
        // 1: id, 2: ownerId, 3: friendId, 4: nickname
        sqlite3_bind_text(stmt, 1, info.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, info.userId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, info.friendId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, info.nickname.c_str(), -1, SQLITE_TRANSIENT);

        // Thực thi câu lệnh
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            TRACE(_T("Lỗi khi thực hiện lưu nickname vào DB: %s\n"), sqlite3_errmsg(m_db));
        }

        // Hủy stmt để giải phóng bộ nhớ
        sqlite3_finalize(stmt);
    }

    ExecuteSQL("COMMIT;");
}


std::vector<Message> DatabaseService::LoadMessages(const std::string& myId, const std::string& friendId) {
    std::vector<Message> messages;

    // Lấy tất cả tin nhắn mà (Tôi gửi cho Bạn) HOẶC (Bạn gửi cho Tôi)
    std::string sqlMsg = "SELECT messageId, senderId, receiverId, content, createdAt, messageType, isSend "
                         "FROM Messages "
                         "WHERE (senderId = ? AND receiverId = ?) "
                         "   OR (senderId = ? AND receiverId = ?) "
                         "ORDER BY createdAt ASC;";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sqlMsg.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        // Bind theo thứ tự: (myId, friendId, friendId, myId)
        sqlite3_bind_text(stmt, 1, myId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, friendId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, friendId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, myId.c_str(), -1, SQLITE_TRANSIENT);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Message msg;
            msg.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            // msg.senderId = ... 
            // msg.receiverId = ...
            msg.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            msg.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            msg.messageType = sqlite3_column_int(stmt, 5);
            msg.isSend = sqlite3_column_int(stmt, 6);

            msg.images = LoadImagesForMessage(msg.id);
            msg.files = LoadFilesForMessage(msg.id);
            messages.push_back(msg);
        }
        sqlite3_finalize(stmt);
    }
    return messages;
}

// Hàm phụ để lấy ảnh
std::vector<MediaItem> DatabaseService::LoadImagesForMessage(const std::string& messageId) {
    std::vector<MediaItem> images;
    std::string sql = "SELECT imageId, url FROM Images WHERE messageId = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, messageId.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            MediaItem img;
            img.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            img.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            images.push_back(img);
        }
        sqlite3_finalize(stmt);
    }
    return images;
}

// Hàm phụ để lấy file
std::vector<MediaItem> DatabaseService::LoadFilesForMessage(const std::string& messageId) {
    std::vector<MediaItem> files;
    std::string sql = "SELECT fileId, url, fileName FROM Files WHERE messageId = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, messageId.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            MediaItem file;
            file.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            file.url = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            file.fileName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            files.push_back(file);
        }
        sqlite3_finalize(stmt);
    }
    return files;
}

bool DatabaseService::SaveUserInfo(const User& user) {
    std::string sql = "INSERT OR REPLACE INTO Users (userId, username, fullName, avatar, password) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user.userId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, user.username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, user.fullName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, user.avatar.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, user.password.c_str(), -1, SQLITE_TRANSIENT);

        bool success = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return success;
    }
    return false;
}

void DatabaseService::EnsureSenderExists(const std::string& senderId) {
    if (senderId == theApp.m_userData.userId) return; // Không cần check chính mình

    // 1. Kiểm tra xem đã tồn tại trong bảng Users chưa
    std::string sqlCheck = "SELECT count(*) FROM Users WHERE userId = ?;";
    sqlite3_stmt* stmtCheck;
    bool exists = false;

    if (sqlite3_prepare_v2(m_db, sqlCheck.c_str(), -1, &stmtCheck, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmtCheck, 1, senderId.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmtCheck) == SQLITE_ROW) {
            if (sqlite3_column_int(stmtCheck, 0) > 0) exists = true;
        }
        sqlite3_finalize(stmtCheck);
    }

    // 2. Nếu chưa có, chèn vào bảng Users với thông tin mặc định
    if (!exists) {
        // Cần đảm bảo đủ cột: userId, username, fullName, password, avatar
        std::string sqlInsert = "INSERT OR IGNORE INTO Users (userId, username, fullName, password, avatar) VALUES (?, ?, ?, ?, ?);";
        sqlite3_stmt* stmtInsert;
        if (sqlite3_prepare_v2(m_db, sqlInsert.c_str(), -1, &stmtInsert, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmtInsert, 1, senderId.c_str(), -1, SQLITE_TRANSIENT);

            // Tạo username duy nhất tránh lỗi UNIQUE
            std::string tempUsername = "user_" + senderId;
            sqlite3_bind_text(stmtInsert, 2, tempUsername.c_str(), -1, SQLITE_TRANSIENT);

            sqlite3_bind_text(stmtInsert, 3, "Unknown User", -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmtInsert, 4, "none", -1, SQLITE_TRANSIENT); // Mật khẩu tạm
            sqlite3_bind_text(stmtInsert, 5, "avatar/default.jpg", -1, SQLITE_TRANSIENT); // Avatar mặc định

            sqlite3_step(stmtInsert);
            sqlite3_finalize(stmtInsert);
        }
    }
}