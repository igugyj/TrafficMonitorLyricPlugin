#pragma once
#include <string>

struct LXStatusData
{
    bool valid{ false };
    std::wstring status;
    std::wstring name;
    std::wstring singer;
    std::wstring lyric_line;
};

class CLXMusicAPI
{
public:
    explicit CLXMusicAPI(const wchar_t* host = L"127.0.0.1", int port = 23330);

    void SetPort(int port) { m_port = port; }
    int GetPort() const { return m_port; }

    void SetTimeout(int timeout_ms) { m_timeout_ms = timeout_ms; }
    int GetTimeout() const { return m_timeout_ms; }

    bool FetchStatus(LXStatusData& out);

private:
    bool HttpGet(const wchar_t* path, std::string& out);

    std::wstring m_host;
    int m_port;
    int m_timeout_ms{ 3000 };
};
