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
        out.lyric_line = Utf8ToWide(j.value("lyricLineText", ""));
        return true;
    }
    catch (...)
    {
        return false;
    }
}

