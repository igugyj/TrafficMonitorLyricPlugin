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
    m_config_path = module_path;
    if (!config_dir.empty())
    {
        size_t index = module_path.find_last_of(L"\\/");
        std::wstring module_file_name = module_path.substr(index + 1);
        size_t dot = module_file_name.find_last_of(L".");
        if (dot != std::wstring::npos)
            module_file_name = module_file_name.substr(0, dot);
        m_config_path = config_dir + module_file_name;
    }
    m_config_path += L".ini";

    m_setting_data.port = GetPrivateProfileInt(L"config", L"port", 23330, m_config_path.c_str());
    m_setting_data.item_width = GetPrivateProfileInt(L"config", L"item_width", 0, m_config_path.c_str());
    m_setting_data.font_size = GetPrivateProfileInt(L"config", L"font_size", 3, m_config_path.c_str());
    m_setting_data.scroll_speed = GetPrivateProfileInt(L"config", L"scroll_speed", 30, m_config_path.c_str());

    wchar_t font_buf[LF_FACESIZE];
    GetPrivateProfileString(L"config", L"font_name", L"Microsoft YaHei UI", font_buf, LF_FACESIZE, m_config_path.c_str());
    m_setting_data.font_name = font_buf;

    m_api.SetPort(m_setting_data.port);
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
    WritePrivateProfileString(L"config", L"font_name", m_setting_data.font_name.c_str(), m_config_path.c_str());
}

void CDataManager::FetchLyric()
{
    LXStatusData data;
    if (m_api.FetchStatus(data))
    {
        m_current_status = data.status;
        m_current_name = data.name;
        m_current_singer = data.singer;

        if (data.lyric_line != m_current_lyric)
        {
            m_current_lyric = data.lyric_line;
            m_lyric_change_time = GetTickCount64();
        }
    }
}
