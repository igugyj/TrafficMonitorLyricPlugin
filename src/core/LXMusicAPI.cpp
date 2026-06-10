#include "pch.h"
#include "LXMusicAPI.h"
#include "json.hpp"
#include <windows.h>
#include <winhttp.h>
#include <memory>

#pragma comment(lib, "winhttp.lib")

struct WinHttpHandleDeleter {
    void operator()(HINTERNET h) const { if (h) WinHttpCloseHandle(h); }
};
using WinHttpHandle = std::unique_ptr<void, WinHttpHandleDeleter>;

static std::wstring Utf8ToWide(const std::string& s)
{
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return {};
    std::wstring w(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], len);
    return w;
}

CLXMusicAPI::CLXMusicAPI(const wchar_t* host, int port)
    : m_host(host), m_port(port) {
}

bool CLXMusicAPI::HttpGet(const wchar_t* path, std::string& out)
{
    WinHttpHandle session(WinHttpOpen(L"LXMusicPlugin/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0));
    if (!session) return false;

    WinHttpHandle connect(WinHttpConnect(session.get(), m_host.c_str(), m_port, 0));
    if (!connect) return false;

    WinHttpHandle request(WinHttpOpenRequest(connect.get(), L"GET", path,
        nullptr, nullptr, nullptr, 0));
    if (!request) return false;

    out.clear();
    char buf[4096];
    if (WinHttpSendRequest(request.get(), nullptr, 0, nullptr, 0, 0, 0) &&
        WinHttpReceiveResponse(request.get(), nullptr))
    {
        DWORD read = 0;
        while (WinHttpReadData(request.get(), buf, sizeof(buf) - 1, &read) && read > 0)
        {
            buf[read] = '\0';
            out += buf;
        }
    }
    return !out.empty();
}

bool CLXMusicAPI::FetchStatus(LXStatusData& out)
{
    std::string raw;
    if (!HttpGet(L"/status", raw)) return false;

    try
    {
        auto j = nlohmann::json::parse(raw);
        out.valid = true;
        out.status = Utf8ToWide(j.value("status", ""));
        out.name = Utf8ToWide(j.value("name", ""));
        out.singer = Utf8ToWide(j.value("singer", ""));
        out.album = Utf8ToWide(j.value("albumName", ""));
        out.lyric_line = Utf8ToWide(j.value("lyricLineText", ""));
        if (j.contains("lyric") && !j["lyric"].is_null())
            out.lyric_raw = Utf8ToWide(j["lyric"].get<std::string>());
        out.progress = j.value("progress", 0.0);
        out.duration = j.value("duration", 0.0);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::vector<LXLrcLine> CLXMusicAPI::ParseLrc(const std::wstring& lrc_text)
{
    std::vector<LXLrcLine> result;
    size_t pos = 0;
    while (pos < lrc_text.size())
    {
        if (lrc_text[pos] != L'[') { pos++; continue; }
        size_t close = lrc_text.find(L']', pos);
        if (close == std::wstring::npos) break;

        std::wstring time_str = lrc_text.substr(pos + 1, close - pos - 1);
        size_t text_start = close + 1;
        size_t text_end = lrc_text.find(L'\n', text_start);
        if (text_end == std::wstring::npos) text_end = lrc_text.size();

        std::wstring text = lrc_text.substr(text_start, text_end - text_start);
        if (!text.empty() && text.back() == L'\r') text.pop_back();
        if (text.empty()) { pos = text_end + 1; continue; }

        int m = 0, s = 0, ms = 0;
        if (swscanf_s(time_str.c_str(), L"%d:%d.%d", &m, &s, &ms) >= 2)
        {
            result.push_back({ m * 60.0 + s + ms * 0.01, text });
        }
        pos = text_end + 1;
    }

    for (size_t i = 1; i < result.size(); i++)
        for (size_t j = i; j > 0 && result[j].time_sec < result[j - 1].time_sec; j--)
            std::swap(result[j], result[j - 1]);

    return result;
}
