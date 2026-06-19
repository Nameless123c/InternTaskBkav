#include "pch.h"
#include "ChangeFormat.h"
#include <string>

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