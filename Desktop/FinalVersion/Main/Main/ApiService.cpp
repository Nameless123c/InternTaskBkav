#include "pch.h"
#include <string>
#include "nlohmann/json.hpp"
#include <curl/curl.h>
#include "ApiService.h"
#include "CHomeChatDlg.h"

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

	FILE* fp = nullptr;
	errno_t err = fopen_s(&fp, savePath.c_str(), "wb");
	if (err != 0 || fp == nullptr) {
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

	fclose(fp); // Đóng file sau khi xong
	if (headers) curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return (res == CURLE_OK);
}

bool ApiService::UpdateProfile(const std::string& url, const std::string& fullName, const std::string& path, const std::string& token) {
	CURL* curl = curl_easy_init();
	if (!curl) return false;

	curl_mime* mime = curl_mime_init(curl);
	curl_mimepart* part = NULL;

	// 1. Thêm trường FullName (Text)
	if (!fullName.empty()) {
		part = curl_mime_addpart(mime);
		curl_mime_name(part, "FullName");
		curl_mime_data(part, fullName.c_str(), CURL_ZERO_TERMINATED);
	}

	// 2. Thêm trường Avatar (File)
	// Server yêu cầu fieldname là 'avatar' (chữ thường)
	if (!path.empty()) {
		part = curl_mime_addpart(mime);
		curl_mime_name(part, "avatar");
		curl_mime_filedata(part, path.c_str());
	}

	// 3. Thiết lập Header
	struct curl_slist* headers = NULL;
	if (!token.empty()) {
		std::string authHeader = "Authorization: Bearer " + token;
		headers = curl_slist_append(headers, authHeader.c_str());
	}

	// 4. Cấu hình cURL
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

	// 5. Thực hiện request
	CURLcode res = curl_easy_perform(curl);

	// 6. Dọn dẹp
	curl_mime_free(mime);
	if (headers) curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return (res == CURLE_OK);
}


std::string ApiService::SendMultipartRequest(const std::string& url,
	const std::string& content,
	const std::string& friendId,
	const std::vector<MediaItem>& files,
	const std::string& token) {
	CURL* curl = curl_easy_init();
	if (!curl) return "";

	std::string readBuffer;
	curl_mime* mime = curl_mime_init(curl);

	// 1. Đóng gói Text Content
	curl_mimepart* part = curl_mime_addpart(mime);
	curl_mime_name(part, "Content");
	curl_mime_data(part, content.c_str(), CURL_ZERO_TERMINATED);

	// 2. Đóng gói FriendID
	part = curl_mime_addpart(mime);
	curl_mime_name(part, "FriendID");
	curl_mime_data(part, friendId.c_str(), CURL_ZERO_TERMINATED);

	// 3. Đóng gói các file đính kèm
	// Lưu ý: Tên field "files" phải khớp chính xác với router.post bên server
	for (const auto& item : files) {
		part = curl_mime_addpart(mime);
		curl_mime_name(part, "files");
		curl_mime_filedata(part, item.url.c_str());
	}

	// 4. Cấu hình Headers (Chỉ cần Authorization, không cần Content-Type)
	struct curl_slist* headers = NULL;
	if (!token.empty()) {
		std::string authHeader = "Authorization: Bearer " + token;
		headers = curl_slist_append(headers, authHeader.c_str());
	}

	// 5. Thiết lập CURL
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime); // Dùng MIMEPOST cho Multipart
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallBack); // Callback của bạn
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // Thời gian chờ upload file có thể lâu hơn

	CURLcode res = curl_easy_perform(curl);

	// 6. Dọn dẹp
	curl_mime_free(mime);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return (res == CURLE_OK) ? readBuffer : "";
}