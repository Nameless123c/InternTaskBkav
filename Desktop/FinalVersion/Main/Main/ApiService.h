#pragma once
#include "pch.h"
#include <string>
#include <nlohmann/json.hpp>
#include "Message.h"

class ApiService {
public:
	ApiService();
	~ApiService();

	static std::string SendPostRequest(const std::string& url, const nlohmann::json& jsonData, const std::string& token = "");
	static std::string SendGetRequest(const std::string& url, const std::string token = "");
	static bool DownloadFile(const std::string& url, const std::string& savePath, const std::string& token);
	static bool UpdateProfile(const std::string& url, const std::string& fullName, const std::string& path, const std::string& token);
	static std::string SendMultipartRequest(const std::string& url, const std::string& content, const std::string& friendId, const std::vector<MediaItem>& files, const std::string& token);
};