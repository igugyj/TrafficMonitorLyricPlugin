#include "pch.h"
#include "LyricPlugin.h"
#include "DataManager.h"

CLyricPlugin CLyricPlugin::m_instance;

CLyricPlugin::CLyricPlugin()
{
}

CLyricPlugin& CLyricPlugin::Instance()
{
    return m_instance;
}

IPluginItem* CLyricPlugin::GetItem(int index)
{
    switch (index)
    {
    case 0:
        return &m_lyric_item;
    default:
        return nullptr;
    }
}

void CLyricPlugin::DataRequired()
{
    CDataManager::Instance().FetchLyric();
}

const wchar_t* CLyricPlugin::GetInfo(PluginInfoIndex index)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    static CString str;
    switch (index)
    {
    case TMI_NAME:
        str.LoadString(IDS_PLUGIN_NAME);
        return str.GetString();
    case TMI_DESCRIPTION:
        str.LoadString(IDS_PLUGIN_DESCRIPTION);
        return str.GetString();
    case TMI_AUTHOR:
        return L"SY Cheng";
    case TMI_COPYRIGHT:
        return L"Copyright (C) 2026";
    case TMI_VERSION:
        return L"" VER_FILE_VERSION_STR;
    case ITMPlugin::TMI_URL:
        return L"https://github.com/igugyj/TrafficMonitorLyricPlugin";
    default:
        break;
    }
    return L"";
}

INT_PTR CALLBACK CLyricPlugin::OptionsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
    {
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        SettingData* data = (SettingData*)lParam;
        SetDlgItemInt(hDlg, IDC_PORT_EDIT, data->port, FALSE);
        SetDlgItemText(hDlg, IDC_FONT_EDIT, data->font_name.c_str());
        SetDlgItemInt(hDlg, IDC_WIDTH_EDIT, data->item_width, FALSE);
        SetDlgItemInt(hDlg, IDC_SPEED_EDIT, data->scroll_speed, FALSE);
        CheckDlgButton(hDlg, IDC_DOUBLE_LINE_CHECK, data->double_line_exclusive ? BST_CHECKED : BST_UNCHECKED);

        HWND hCombo = GetDlgItem(hDlg, IDC_FONT_COMBO);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"小");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"中");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"大");
        SendMessage(hCombo, CB_SETCURSEL, data->font_size - 2, 0);
        return TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_FONT_BUTTON:
        {
            SettingData* data = (SettingData*)GetWindowLongPtr(hDlg, DWLP_USER);

            wchar_t current_font[LF_FACESIZE];
            GetDlgItemText(hDlg, IDC_FONT_EDIT, current_font, LF_FACESIZE);

            LOGFONTW lf = { 0 };
            wcscpy_s(lf.lfFaceName, current_font);
            lf.lfHeight = -MulDiv(12, 96, 72);
            lf.lfWeight = FW_NORMAL;
            lf.lfCharSet = DEFAULT_CHARSET;

            CHOOSEFONTW cf = { sizeof(CHOOSEFONTW) };
            cf.hwndOwner = hDlg;
            cf.lpLogFont = &lf;
            cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
            cf.nFontType = REGULAR_FONTTYPE;

            if (ChooseFont(&cf))
            {
                SetDlgItemText(hDlg, IDC_FONT_EDIT, lf.lfFaceName);
                data->font_name = lf.lfFaceName;
            }
            return TRUE;
        }
        case IDOK:
        {
            SettingData* data = (SettingData*)GetWindowLongPtr(hDlg, DWLP_USER);
            BOOL translated;
            data->port = GetDlgItemInt(hDlg, IDC_PORT_EDIT, &translated, FALSE);
            data->item_width = GetDlgItemInt(hDlg, IDC_WIDTH_EDIT, &translated, FALSE);
            if (data->item_width < 0) data->item_width = 0;
            data->scroll_speed = GetDlgItemInt(hDlg, IDC_SPEED_EDIT, &translated, FALSE);
            if (data->scroll_speed < 0) data->scroll_speed = 0;
            data->double_line_exclusive = (IsDlgButtonChecked(hDlg, IDC_DOUBLE_LINE_CHECK) == BST_CHECKED);
            HWND hCombo = GetDlgItem(hDlg, IDC_FONT_COMBO);
            data->font_size = (int)SendMessage(hCombo, CB_GETCURSEL, 0, 0) + 2;
            wchar_t font_buf[LF_FACESIZE];
            GetDlgItemText(hDlg, IDC_FONT_EDIT, font_buf, LF_FACESIZE);
            data->font_name = font_buf;
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

ITMPlugin::OptionReturn CLyricPlugin::ShowOptionsDialog(void* hParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    SettingData data = CDataManager::Instance().m_setting_data;
    INT_PTR ret = DialogBoxParam(AfxGetResourceHandle(), MAKEINTRESOURCE(IDD_OPTIONS_DIALOG),
        (HWND)hParent, OptionsDlgProc, (LPARAM)&data);
    if (ret == IDOK)
    {
        CDataManager::Instance().m_setting_data = data;
        CDataManager::Instance().SaveConfig();
        CDataManager::Instance().ApplySettings();
        return ITMPlugin::OR_OPTION_CHANGED;
    }
    return ITMPlugin::OR_OPTION_UNCHANGED;
}

void CLyricPlugin::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case ITMPlugin::EI_CONFIG_DIR:
        CDataManager::Instance().LoadConfig(std::wstring(data));
        break;
    default:
        break;
    }
}

const wchar_t* CLyricPlugin::GetTooltipInfo()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    const auto& data = CDataManager::Instance();
    const std::wstring& status = data.GetCurrentStatus();
    if (status == L"playing" || status == L"paused")
    {
        m_tooltip = data.GetCurrentName();
        if (!data.GetCurrentSinger().empty())
            m_tooltip += L" - " + data.GetCurrentSinger();
        if (m_tooltip.empty())
            m_tooltip = data.GetCurrentLyric();
    }
    else
    {
        m_tooltip = L"未在播放";
    }
    return m_tooltip.c_str();
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CLyricPlugin::Instance();
}

//////////////////////////////////////////////////////////////////////////
// CLyricItem

const wchar_t* CLyricPlugin::CLyricItem::GetItemName() const
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    static CString str;
    str.LoadString(IDS_LYRIC_ITEM_NAME);
    return str.GetString();
}

const wchar_t* CLyricPlugin::CLyricItem::GetItemId() const
{
    return L"LyricPlugin";
}

const wchar_t* CLyricPlugin::CLyricItem::GetItemLableText() const
{
    return L"";
}

const wchar_t* CLyricPlugin::CLyricItem::GetItemValueText() const
{
    return CDataManager::Instance().GetCurrentLyric().c_str();
}

const wchar_t* CLyricPlugin::CLyricItem::GetItemValueSampleText() const
{
    return L"Hello World";
}

bool CLyricPlugin::CLyricItem::IsCustomDraw() const
{
    return true;
}

int CLyricPlugin::CLyricItem::IsDoubleLineExclusive() const
{
    return CDataManager::Instance().m_setting_data.double_line_exclusive ? 1 : 0;
}

int CLyricPlugin::CLyricItem::GetItemWidthEx(void* hDC) const
{
    const std::wstring& status = CDataManager::Instance().GetCurrentStatus();
    if (status != L"playing") return 0;

    int item_width = CDataManager::Instance().m_setting_data.item_width;
    if (item_width > 0) return item_width;

    const std::wstring& text = CDataManager::Instance().GetCurrentLyric();
    if (text.empty()) return 0;

    CDC* pDC = CDC::FromHandle((HDC)hDC);
    SettingData& setting = CDataManager::Instance().m_setting_data;

    double font_ratio = 0.75;
    if (setting.font_size == 2) font_ratio = 0.6;
    else if (setting.font_size == 4) font_ratio = 0.9;

    int height = m_cache_height > 0 ? m_cache_height : 32;
    CFont* p_font = nullptr;
    CFont temp_font;
    if (setting.font_size == m_cache_font_size && height == m_cache_height && setting.font_name == m_cache_font_name)
    {
        p_font = &m_font;
    }
    else
    {
        LOGFONTW lf = { 0 };
        lf.lfHeight = -(int)(height * font_ratio);
        lf.lfWeight = FW_NORMAL;
        lf.lfQuality = CLEARTYPE_QUALITY;
        wcscpy_s(lf.lfFaceName, setting.font_name.c_str());
        temp_font.CreateFontIndirectW(&lf);
        p_font = &temp_font;
    }

    CFont* old_font = pDC->SelectObject(p_font);

    CSize size = pDC->GetTextExtent(text.c_str(), (int)text.size());

    pDC->SelectObject(old_font);
    return size.cx + 8;
}

void CLyricPlugin::CLyricItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    const std::wstring& text = CDataManager::Instance().GetCurrentLyric();
    if (text.empty()) return;

    int saved = pDC->SaveDC();

    SettingData& setting = CDataManager::Instance().m_setting_data;
    int font_size = setting.font_size;
    int scroll_speed = setting.scroll_speed;

    double font_ratio = 0.75;
    if (font_size == 2) font_ratio = 0.6;
    else if (font_size == 4) font_ratio = 0.9;

    if (font_size != m_cache_font_size || h != m_cache_height || setting.font_name != m_cache_font_name)
    {
        LOGFONTW lf = { 0 };
        lf.lfHeight = -(int)(h * font_ratio);
        lf.lfWeight = FW_NORMAL;
        lf.lfQuality = CLEARTYPE_QUALITY;
        wcscpy_s(lf.lfFaceName, setting.font_name.c_str());
        m_font.DeleteObject();
        m_font.CreateFontIndirectW(&lf);
        m_cache_font_size = font_size;
        m_cache_height = h;
        m_cache_font_name = setting.font_name;
    }
    pDC->SelectObject(&m_font);

    COLORREF color = dark_mode ? RGB(230, 230, 230) : RGB(30, 30, 30);
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(color);

    CSize size = pDC->GetTextExtent(text.c_str(), (int)text.size());
    int text_width = size.cx;
    int y_center = y + (h - size.cy) / 2;

    pDC->IntersectClipRect(x, y, x + w, y + h);

    if (text_width <= w || scroll_speed == 0)
    {
        pDC->TextOutW(x, y_center, text.c_str(), (int)text.size());
    }
    else
    {
        int scrollable = text_width - w;
        int idle_ms = 500;
        int pause_ms = 500;
        int scroll_ms = scroll_speed > 0
            ? (int)((double)scrollable / scroll_speed * 1000.0)
            : 1;
        int total_ms = idle_ms + scroll_ms + pause_ms;

        ULONGLONG elapsed = GetTickCount64() - CDataManager::Instance().GetLyricChangeTime();
        int phase = (int)(elapsed % total_ms);

        int offset = 0;
        if (phase >= idle_ms && phase < idle_ms + scroll_ms)
            offset = (int)((double)(phase - idle_ms) * scroll_speed / 1000.0);
        else if (phase >= idle_ms + scroll_ms)
            offset = scrollable;

        int draw_x = x - offset;
        pDC->TextOutW(draw_x, y_center, text.c_str(), (int)text.size());
    }

    pDC->RestoreDC(saved);
}
