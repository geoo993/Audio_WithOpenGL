
#include "GameManager.h"


CGameManager::CGameManager() : resourcePath(""), /*resourceType(RESOURCE_NULL),*/ isLoaded(false), isActive(false) {}
CGameManager::~CGameManager() {}

void CGameManager::SetResourcePath(const string &filepath) {
    resourcePath = filepath;
}

string CGameManager::GetResourcePath() const {
    return resourcePath;
}

void CGameManager::SetLoaded(const bool &value) {
    isLoaded = value;
}

bool CGameManager::IsLoaded() const {
    return isLoaded;
}

void CGameManager::SetActive(const bool &value) {
    isActive = value;
}
bool CGameManager::IsActive() const {
    return isActive;
}
