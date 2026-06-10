#pragma once
#include <string>
#include <vector>

struct LXStatusData
{
    bool valid{ false };
    std::wstring status;
    std::wstring name;
    std::wstring singer;
    std::wstring album;
    std::wstring lyric_line;
    std::wstring lyric_raw;
    double progress{ 0 };
    double duration{ 0 };
};

struct LXLrcLine
{
    double time_sec{ 0 };
    std::wstring text;
};

class CLXMusicAPI
{
public:
    explicit CLXMusicAPI(const wchar_t* host = L"127.0.0.1", int port = 23330);

    void SetPort(int port) { m_port = port; }
    int GetPort() const { return m_port; }

    bool FetchStatus(LXStatusData& out);

    static std::vector<LXLrcLine> ParseLrc(const std::wstring& lrc_text);

private:
    bool HttpGet(const wchar_t* path, std::string& out);

    std::wstring m_host;
    int m_port;
};
