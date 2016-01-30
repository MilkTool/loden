#include "Loden/Settings.hpp"
#include <stdlib.h>
#include <stdio.h>

namespace Loden
{

SettingsCategory::SettingsCategory()
{
}

SettingsCategory::~SettingsCategory()
{
}

bool SettingsCategory::hasValue(const std::string &name) const
{
    std::unique_lock<std::mutex>(mutex);
    auto it = keyValues.find(name);
    return it != keyValues.end();
}

int SettingsCategory::getBoolValue(const std::string &name, bool defaultValue) const
{
    std::unique_lock<std::mutex>(mutex);
    auto it = keyValues.find(name);
    if (it != keyValues.end())
        return it->second != "0" && it->second != "false";
    return defaultValue;
}

int SettingsCategory::getIntValue(const std::string &name, int defaultValue) const
{
    std::unique_lock<std::mutex>(mutex);
    auto it = keyValues.find(name);
    if (it != keyValues.end())
        return atoi(it->second.c_str());
    return defaultValue;
}

double SettingsCategory::getFloatValue(const std::string &name, double defaultValue) const
{
    std::unique_lock<std::mutex>(mutex);
    auto it = keyValues.find(name);
    if (it != keyValues.end())
        return atof(it->second.c_str());
    return defaultValue;
}

std::string SettingsCategory::getStringValue(const std::string &name, const std::string &defaultValue) const
{
    std::unique_lock<std::mutex>(mutex);
    auto it = keyValues.find(name);
    if (it != keyValues.end())
        return it->second;
    return defaultValue;
}

void SettingsCategory::setBoolValue(const std::string &name, bool value)
{
    std::unique_lock<std::mutex>(mutex);
    keyValues[name] = value ? "true" : "false";
}

void SettingsCategory::setIntValue(const std::string &name, int value)
{
    char buffer[64];
    sprintf(buffer, "%d", value);
    setStringValue(name, buffer);
}

void SettingsCategory::setFloatValue(const std::string &name, double value)
{
    char buffer[64];
    sprintf(buffer, "%f", value);
    setStringValue(name, buffer);
}

void SettingsCategory::setStringValue(const std::string &name, const std::string &value)
{
    std::unique_lock<std::mutex>(mutex);
    keyValues[name] = value;
}

Settings::Settings()
{
}

Settings::~Settings()
{
}

SettingsCategoryPtr Settings::getOrCreateCategory(const std::string &categoryName)
{
    std::unique_lock<std::mutex> (mutex);
    auto it = categories.find(categoryName);
    if (it != categories.end())
        return it->second;
    
    auto category = std::make_shared<SettingsCategory>();
    categories.insert(std::make_pair(categoryName, category));
    return category;
}

SettingsCategoryPtr Settings::getCategory(const std::string &categoryName) const
{
    std::unique_lock<std::mutex>(mutex);
    auto it = categories.find(categoryName);
    if (it != categories.end())
        return it->second;
    return nullptr;
}

bool Settings::hasValue(const std::string &categoryName, const std::string &name) const
{
    auto category = getCategory(categoryName);
    if (category)
        return category->hasValue(name);
    return false;
}

int Settings::getBoolValue(const std::string &categoryName, const std::string &name, bool defaultValue) const
{
    auto category = getCategory(categoryName);
    if (category)
        return category->getBoolValue(name, defaultValue);
    return defaultValue;
}

int Settings::getIntValue(const std::string &categoryName, const std::string &name, int defaultValue) const
{
    auto category = getCategory(categoryName);
    if (category)
        return category->getIntValue(name, defaultValue);
    return defaultValue;
}

double Settings::getFloatValue(const std::string &categoryName, const std::string &name, double defaultValue) const
{
    auto category = getCategory(categoryName);
    if (category)
        return category->getFloatValue(name, defaultValue);
    return defaultValue;
}

std::string Settings::getStringValue(const std::string &categoryName, const std::string &name, const std::string &defaultValue) const
{
    auto category = getCategory(categoryName);
    if (category)
        return category->getStringValue(name, defaultValue);
    return defaultValue;
}

void Settings::setBoolValue(const std::string &categoryName, const std::string &name, bool value)
{
    auto category = getOrCreateCategory(categoryName);
    category->setBoolValue(name, value);
}

void Settings::setIntValue(const std::string &categoryName, const std::string &name, int value)
{
    auto category = getOrCreateCategory(categoryName);
    category->setIntValue(name, value);
}

void Settings::setFloatValue(const std::string &categoryName, const std::string &name, double value)
{
    auto category = getOrCreateCategory(categoryName);
    category->setFloatValue(name, value);
}

void Settings::setStringValue(const std::string &categoryName, const std::string &name, const std::string &value)
{
    auto category = getOrCreateCategory(categoryName);
    category->setStringValue(name, value);
}

} // End of namespace Loden
