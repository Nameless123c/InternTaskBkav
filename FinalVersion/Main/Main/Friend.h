#pragma once
#include <afx.h>
#include <atltime.h> 
#include <string>

struct Friend {
    std::string friendId;
    std::string fullName;
    std::string avatar;
    bool isOnline;

    CImage* pAvatar;

    Friend();
};