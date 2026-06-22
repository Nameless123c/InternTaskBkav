#pragma once
#include "pch.h"
#include <string>
#include <nlohmann/json.hpp>

class ApiService {
public:
	ApiService();
	~ApiService();

	static std::string SendPostRequest(const std::string& url, const nlohmann::json& jsonData, const std::string& token = "");
	static std::string SendGetRequest(const std::string& url, const std::string token = "");
	static bool DownloadFile(const std::string& url, const std::string& savePath, const std::string& token);
};