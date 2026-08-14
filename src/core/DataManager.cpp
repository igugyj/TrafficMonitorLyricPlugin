#include "pch.h"
#include "DataManager.h"

CDataManager CDataManager::m_instance;

CDataManager::CDataManager()
{
}

CDataManager::~CDataManager()
{
    SaveConfig();
}

CDataManager& CDataManager::Instance()
{
    return m_instance;
}

void CDataManager::LoadConfig(const std::wstring& config_dir)
{
    HMODULE hModule = reinterpret_cast<HMODULE>(&__ImageBase);
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(hModule, path, MAX_PATH);
    std::wstring module_path = path;
    if (!config_dir.empty())
    {
        m_config_path = config_dir + L"LyricPlugin.ini";
    }
    else
    {
        size_t slash = module_path.find_last_of(L"\\/");
        m_config_path = module_path.substr(0, slash + 1) + L"LyricPlugin.ini";
    }

    m_setting_data.port = GetPrivateProfileInt(L"config", L"port", 23330, m_config_path.c_str());
    m_setting_data.item_width = GetPrivateProfileInt(L"config", L"item_width", 0, m_config_path.c_str());
    m_setting_data.font_size = GetPrivateProfileInt(L"config", L"font_size", 3, m_config_path.c_str());
    m_setting_data.scroll_speed = GetPrivateProfileInt(L"config", L"scroll_speed", 30, m_config_path.c_str());
    m_setting_data.timeout_ms = GetPrivateProfileInt(L"config", L"timeout", 3000, m_config_path.c_str());

    wchar_t font_buf[LF_FACESIZE];
    GetPrivateProfileString(L"config", L"font_name", L"Microsoft YaHei UI", font_buf, LF_FACESIZE, m_config_path.c_str());
    m_setting_data.font_name = font_buf;

    if (m_setting_data.port < 1 || m_setting_data.port > 65535) m_setting_data.port = 23330;
    if (m_setting_data.item_width < 0) m_setting_data.item_width = 0;
    if (m_setting_data.font_size < 2 || m_setting_data.font_size > 4) m_setting_data.font_size = 3;
    if (m_setting_data.scroll_speed < 0) m_setting_data.scroll_speed = 30;
    if (m_setting_data.timeout_ms < 100 || m_setting_data.timeout_ms > 60000) m_setting_data.timeout_ms = 3000;

    m_api.SetPort(m_setting_data.port);
    m_api.SetTimeout(m_setting_data.timeout_ms);
}

void CDataManager::ApplySettings()
{
    m_api.SetPort(m_setting_data.port);
    m_api.SetTimeout(m_setting_data.timeout_ms);
}

void CDataManager::SaveConfig() const
{
    wchar_t buff[32];
    swprintf_s(buff, L"%d", m_setting_data.port);
    WritePrivateProfileString(L"config", L"port", buff, m_config_path.c_str());
    swprintf_s(buff, L"%d", m_setting_data.item_width);
    WritePrivateProfileString(L"config", L"item_width", buff, m_config_path.c_str());
    swprintf_s(buff, L"%d", m_setting_data.font_size);
    WritePrivateProfileString(L"config", L"font_size", buff, m_config_path.c_str());
    swprintf_s(buff, L"%d", m_setting_data.scroll_speed);
    WritePrivateProfileString(L"config", L"scroll_speed", buff, m_config_path.c_str());
    swprintf_s(buff, L"%d", m_setting_data.timeout_ms);
    WritePrivateProfileString(L"config", L"timeout", buff, m_config_path.c_str());
    WritePrivateProfileString(L"config", L"font_name", m_setting_data.font_name.c_str(), m_config_path.c_str());
}

void CDataManager::FetchLyric()
{
    LXStatusData data;
    if (m_api.FetchStatus(data))
    {
        m_fail_count = 0;
        m_pending_status = data.status;
        m_pending_name = data.name;
        m_pending_singer = data.singer;
        m_pending_lyric = data.lyric_line;
        m_pending_valid = true;
    }
    else if (++m_fail_count >= 2)
    {
        m_pending_status.clear();
        m_pending_name.clear();
        m_pending_singer.clear();
        m_pending_lyric.clear();
        m_pending_valid = true;
    }
}

void CDataManager::CommitPending()
{
    if (!m_pending_valid) return;
    m_pending_valid = false;
    m_current_status = m_pending_status;
    m_current_name = m_pending_name;
    m_current_singer = m_pending_singer;
    if (m_pending_lyric != m_current_lyric)
    {
        m_current_lyric = m_pending_lyric;
        m_lyric_change_time = GetTickCount64();
    }
}
