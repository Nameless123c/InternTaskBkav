#include "pch.h"
#include <string>

#pragma once
class ChangeFormat {
public:
	ChangeFormat();
	~ChangeFormat();

	static std::string CStringToUTF8(CString str);
	static CString UTF8ToCString(const std::string& str);
	static CString FormatISOToDateTime(std::string isoTime);
};