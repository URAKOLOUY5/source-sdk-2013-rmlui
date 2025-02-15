// ==================================================================
// Den Urakolouy AKA URAKOLOUY5
// 2025
// 
// Feel free to use it as you want to use.
// Major code based on open source references from Source SDK.
// ==================================================================

#ifndef RMLPANEL_H
#define RMLPANEL_H

#include "cbase.h"
#include "vgui_controls/Panel.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class RmlUiPanel : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(RmlUiPanel, vgui::Panel);

public:
    RmlUiPanel();
    virtual void OnScreenSizeChanged(int iOldWide, int iOldTall) override;
    virtual void OnCursorMoved(int x, int y) override;
    virtual void OnMousePressed(vgui::MouseCode code) override;
    virtual void OnMouseDoublePressed(vgui::MouseCode code) override;
    virtual void OnMouseReleased(vgui::MouseCode code) override;
    virtual void OnMouseWheeled(int delta);
    virtual void OnKeyCodePressed(vgui::KeyCode code);
    virtual void OnKeyCodeReleased(vgui::KeyCode code);
    virtual void OnKeyTyped(wchar_t unichar);;
    virtual void Paint() override;
    virtual void PaintBackground() override;
};

#endif // RMLPANEL_H