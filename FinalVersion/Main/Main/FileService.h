#pragma once
#include "pch.h"
#include <string>
#include <gdiplus.h>

class FileService {
public:
    static CImage* LoadImageFromFile(const std::string& path);
};