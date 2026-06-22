#include "pch.h"
#include <string>
#include "nlohmann/json.hpp"
#include <curl/curl.h>
#include "ApiService.h"

size_t WriteCallBack(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t total_size = size * nmemb;

	std::string* str = static_cast<std::string*>(userp);
	
	str->append(static_cast<const char*>(contents), total_size);

	return total_size;
}

std::string ApiService::SendGetRequest(const std::string& url, const std::string token) {
	CURL* curl = curl_easy_init();

	if (!curl) return "";

	std::string readBuffer;

	struct curl_slist* headers = NULL;

	if (!token.empty()) {
		std::string authHeader = "Authorization: Bearer " + token;
		headers = curl_slist_append(headers, authHeader.c_str());
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallBack);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L); // chờ TCP handshake 5s
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L); // tổng TG chờ 10s

	CURLcode res = curl_easy_perform(curl);

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return (res == CURLE_OK) ? readBuffer : "";
}

std::string ApiService::SendPostRequest(const std::string& url, const nlohmann::json& jsonData, const std::string& token) {
	CURL* curl = curl_easy_init();

	if (!curl) return "";

	std::string readBuffer;
	std::string jsonString = jsonData.dump();

	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");

	if (!token.empty()) {
		std::string authHeader = "Authorization: Bearer " + token;
		headers = curl_slist_append(headers, authHeader.c_str());
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallBack);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L); // chờ TCP handshake 5s
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L); // tổng TG chờ 10s

	CURLcode res = curl_easy_perform(curl);

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);


	return (res == CURLE_OK) ? readBuffer : "";
}

size_t WriteFileCallBack(void* contents, size_t size, size_t nmemb, void* userp) {
	FILE* file = static_cast<FILE*>(userp);
	return fwrite(contents, size, nmemb, file);
}

bool ApiService::DownloadFile(const std::string& url, const std::string& savePath, const std::string& token) {
	CURL* curl = curl_easy_init();
	if (!curl) return false;

	FILE* fp = fopen(savePath.c_str(), "wb");
	if (!fp) {
		curl_easy_cleanup(curl);
		return false;
	}

	struct curl_slist* headers = NULL;
	if (!token.empty()) {
		std::string authHeader = "Authorization: Bearer " + token;
		headers = curl_slist_append(headers, authHeader.c_str());
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallBack);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); 
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3600L);

	CURLcode res = curl_easy_perform(curl);

	fclose(fp); // Đóng file sau khi ghi xong
	if (headers) curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return (res == CURLE_OK);
}