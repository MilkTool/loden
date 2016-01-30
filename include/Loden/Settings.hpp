#ifndef LODEN_SETTINGS_HPP_
#define LODEN_SETTINGS_HPP_

#include "Loden/Object.hpp"
#include <unordered_map>
#include <string>
#include <mutex>

namespace Loden
{

LODEN_DECLARE_CLASS(Settings);
LODEN_DECLARE_CLASS(SettingsCategory);

/**
 * Settings category
 */
class LODEN_CORE_EXPORT SettingsCategory : public Object
{
public:
    SettingsCategory();
    ~SettingsCategory();

    bool hasValue(const std::string &name) const;
    int getBoolValue(const std::string &name, bool defaultValue) const;
    int getIntValue(const std::string &name, int defaultValue = 0) const;
    double getFloatValue(const std::string &name, double defaultValue = 0) const;
    std::string getStringValue(const std::string &name, const std::string &defaultValue = std::string()) const;

    void setBoolValue(const std::string &name, bool value);
    void setIntValue(const std::string &name, int value);
    void setFloatValue(const std::string &name, double value);
    void setStringValue(const std::string &name, const std::string &value);

private:
    std::mutex mutex;
    std::unordered_map<std::string, std::string> keyValues;
};

/**
 * Settings.
 */
class LODEN_CORE_EXPORT Settings: public Object
{
public:
    Settings();
    ~Settings();

    SettingsCategoryPtr getOrCreateCategory(const std::string &categoryName);
    SettingsCategoryPtr getCategory(const std::string &categoryName) const;

    bool hasValue(const std::string &categoryName, const std::string &name) const;
    int getBoolValue(const std::string &categoryName, const std::string &name, bool defaultValue) const;
    int getIntValue(const std::string &categoryName, const std::string &name, int defaultValue = 0) const;
    double getFloatValue(const std::string &categoryName, const std::string &name, double defaultValue = 0) const;
    std::string getStringValue(const std::string &categoryName, const std::string &name, const std::string &defaultValue = std::string()) const;

    void setBoolValue(const std::string &categoryName, const std::string &name, bool value);
    void setIntValue(const std::string &categoryName, const std::string &name, int value);
    void setFloatValue(const std::string &categoryName, const std::string &name, double value);
    void setStringValue(const std::string &categoryName, const std::string &name, const std::string &value);

private:
    std::mutex mutex;
    std::unordered_map<std::string, SettingsCategoryPtr> categories;

};
} // End of namespace Loden

#endif //LODEN_SETTINGS_HPP_
