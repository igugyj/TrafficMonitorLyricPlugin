#pragma once
#include <string>
#include "PluginInterface.h"

class CLyricPlugin : public ITMPlugin
{
private:
    CLyricPlugin();

    class CLyricItem : public IPluginItem
    {
    public:
        virtual const wchar_t* GetItemName() const override;
        virtual const wchar_t* GetItemId() const override;
        virtual const wchar_t* GetItemLableText() const override;
        virtual const wchar_t* GetItemValueText() const override;
        virtual const wchar_t* GetItemValueSampleText() const override;
        virtual bool IsCustomDraw() const override;
        virtual int GetItemWidthEx(void* hDC) const override;
        virtual void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;
    };

public:
    static CLyricPlugin& Instance();

    virtual IPluginItem* GetItem(int index) override;
    virtual void DataRequired() override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual OptionReturn ShowOptionsDialog(void* hParent) override;
    virtual void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;
    virtual void OnInitialize(ITrafficMonitor* pApp) override;
    virtual const wchar_t* GetTooltipInfo() override;

private:
    CLyricItem m_lyric_item;
    ITrafficMonitor* m_app{};
    std::wstring m_tooltip;
    static CLyricPlugin m_instance;

    static INT_PTR CALLBACK OptionsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();
#ifdef __cplusplus
}
#endif
