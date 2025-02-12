// ==================================================================
// Den Urakolouy AKA URAKOLOUY5
// 2025
// 
// Feel free to use it as you want to use.
// Major code based on open source references from Source SDK.
// ==================================================================

#ifndef RMLUIMANAGER_H
#define RMLUIMANAGER_H

// NOTE: Source SDK's Assert breaks RmlUi
// undef it, import RmlUi stuff
// and redefine it back to dbg.h version
#ifdef Assert
#undef Assert
#endif

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Debugger.h>

#ifdef DBGFLAG_ASSERT
#define  Assert( _exp )           							_AssertMsg( _exp, _T("Assertion Failed: ") _T(#_exp), ((void)0), false )
#else
#define  Assert( _exp )										((void)0)
#endif

#include "kbutton.h"
#include <vgui_controls/Controls.h>
#include <vgui/IInput.h>

#include "rmlui_panel.h"

class RmlUIManager {
private:
    Rml::Context* pContext;
    static RmlUIManager* instance;
    RmlUiPanel* rmlPanel;
    
    RmlUIManager();
    void LoadFontFaces();

public:
    RmlUIManager(const RmlUIManager&) = delete;
    RmlUIManager& operator=(const RmlUIManager&) = delete;
    ~RmlUIManager();

    static RmlUIManager* GetInstance();
    void Init();
    void Render();
    void OnScreenSizeChanged(int iOldWide, int iOldTall);
    void OnCursorMoved(int x, int y);
    void OnMousePressed(vgui::MouseCode code);
    void OnMouseDoublePressed(vgui::MouseCode code);
    void OnMouseReleased(vgui::MouseCode code);
    void OnMouseWheeled(int delta);
    void OnKeyCodePressed(ButtonCode_t keynum);
    void OnKeyCodeReleased(ButtonCode_t keynum);
    void OnKeyTyped(wchar_t unichar);
    void SetInputEnabled(bool state);
    void Shutdown();
};

#endif // RMLUIMANAGER_H
