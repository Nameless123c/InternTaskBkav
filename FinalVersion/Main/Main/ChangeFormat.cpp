#include "pch.h"
#include "ChangeFormat.h"
#include <string>

std::string ChangeFormat::CStringToUTF8(CString str) {
	str.Trim();
	if (str.IsEmpty()) return "";
	int len = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	if (len <= 1) return "";
	std::string strUTF8(len - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, str, -1, &strUTF8[0], len - 1, NULL, NULL);
	return strUTF8;
}

CString ChangeFormat::UTF8ToCString(const std::string& str) {
	if (str.empty()) return _T("");
	int wLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	if (wLen == 0) return _T("");
	std::wstring wStr(wLen, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wStr[0], wLen);
	return CString(wStr.c_str());
}

CString ChangeFormat::FormatISOToDateTime(std::string isoTime) {

    int year = std::stoi(isoTime.substr(0, 4));
    int month = std::stoi(isoTime.substr(5, 2));
    int day = std::stoi(isoTime.substr(8, 2));
    int hour = std::stoi(isoTime.substr(11, 2));
    int minute = std::stoi(isoTime.substr(14, 2));
    int second = std::stoi(isoTime.substr(17, 2));

    hour += 7;
    if (hour >= 24) {
        hour -= 24;
        day += 1; 
    }

    CString strResult;
    strResult.Format(_T("%02d/%02d/%04d %02d:%02d:%02d"), day, month, year, hour, minute, second);
    return strResult;
}