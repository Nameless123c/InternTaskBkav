#include "pch.h"
#include <string>

#pragma once
class ChangeFormat {
public:
	ChangeFormat();
	~ChangeFormat();

	static CString FormatISOToDateTime(std::string isoTime);
};