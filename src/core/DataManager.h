#pragma once
#include <string>
#include "LXMusicAPI.h"

struct SettingData
{
    int port{ 23330 };
    int item_width{ 0 };
    int font_size{ 3 };
    int scroll_speed{ 30 };
    int timeout_ms{ 3000 };
    std::wstring font_name{ L"Microsoft YaHei UI" };
};

class CDataManager
{
private:
    CDataManager();
    ~CDataManager();

public:
    static CDataManager& Instance();

    void LoadConfig(const std::wstring& config_dir);
    void SaveConfig() const;
    void ApplySettings();
    void FetchLyric();

    const std::wstring& GetCurrentLyric() const { return m_current_lyric; }
    const std::wstring& GetCurrentStatus() const { return m_current_status; }
    const std::wstring& GetCurrentName() const { return m_current_name; }
    const std::wstring& GetCurrentSinger() const { return m_current_singer; }
    ULONGLONG GetLyricChangeTime() const { return m_lyric_change_time; }

    SettingData m_setting_data;

private:
    static CDataManager m_instance;
    std::wstring m_config_path;
    std::wstring m_current_lyric;
    std::wstring m_current_status;
    std::wstring m_current_name;
    std::wstring m_current_singer;
    ULONGLONG m_lyric_change_time{ 0 };
    CLXMusicAPI m_api;
};
