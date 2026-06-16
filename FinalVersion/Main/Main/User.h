#pragma once
#include <afx.h>
#include <atltime.h> 
#include <string>

struct User {
    std::string fullName;
    std::string username;
    std::string password;
    std::string avatar;
    
    CImage* pAvatar;
    std::string token;

    User();
};