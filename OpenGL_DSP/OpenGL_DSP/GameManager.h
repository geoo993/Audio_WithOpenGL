#pragma once
#include "Common.h"

class CGameManager
{
    
public:
    
    CGameManager();
    virtual ~CGameManager();
    
    void SetResourcePath(const std::string &filepath);
    std::string GetResourcePath() const;

    void SetLoaded(const bool &value);
    bool IsLoaded() const;
    
    void SetActive(const bool &value);
    bool IsActive() const;
    
private:
    
//protected:
    std::string resourcePath;
    bool isLoaded;
    bool isActive;
    
};

