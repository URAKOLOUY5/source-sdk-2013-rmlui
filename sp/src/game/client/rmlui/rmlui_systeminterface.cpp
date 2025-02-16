// ==================================================================
// Den Urakolouy AKA URAKOLOUY5
// 2025
// 
// Feel free to use it as you want to use.
// Major code based on open source references from Source SDK.
// ==================================================================

#include "cbase.h"
#include "rmlui_systeminterface.h"
#include "vgui/IInput.h"
#include "vgui/Cursor.h"
#include "vgui_controls/Controls.h"
#include <locale>
#include <codecvt>

#ifdef WIN32
#pragma warning(push)
#pragma warning(disable: 4005)
#include <Windows.h>
#pragma warning(pop)
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/// Log messages from RmlUi
bool RmlUiSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
	switch (type) {
        case Rml::Log::Type::LT_WARNING:
            Warning("[Rml Ui Warning] %s\n", message.c_str());
#ifdef WIN32
            MessageBox(NULL, message.c_str(), "RmlUi Warning", MB_OK | MB_ICONWARNING);
#else
#error Add support for message boxes on other non windows systems!
#endif
            break;
		case Rml::Log::Type::LT_ERROR:
			Warning("[Rml Ui Error] %s\n", message.c_str());
#ifdef WIN32
            MessageBox(NULL, message.c_str(), "RmlUi Error", MB_OK | MB_ICONERROR);
#else
#error Add support for message boxes on other non windows systems!
#endif
            __debugbreak();
			break;
		case Rml::Log::Type::LT_ASSERT:
			Msg("[Rml Ui Assert] %s\n", message.c_str());
			Error("RmlUI : %s", message.c_str());
			break;
		default:
			ConColorMsg(Color(255,220,0,255), "[Rml Ui] %s\n", message.c_str());
			break;
	}

	return true;
}

/// Reroute paths (if path starts with mod:// look up file in MOD search path
/// otherwise use internal rmlui logic
void RmlUiSystemInterface::JoinPath(Rml::String& translated_path, const Rml::String& document_path, const Rml::String& path)
{
    const Rml::String allowed_protocol = "mod://";

    if (path.compare(0, allowed_protocol.length(), allowed_protocol) == 0)
        translated_path = path.substr(allowed_protocol.length());
    else
        SystemInterface::JoinPath(translated_path, document_path, path);
}

/// Called when RmlUi wants to set cursor
void RmlUiSystemInterface::SetMouseCursor(const Rml::String& cursor_name)
{
    vgui::HCursor cursor_code = vgui::CursorCode::dc_arrow; // Default to arrow cursor

    if (cursor_name == "arrow")
        cursor_code = vgui::CursorCode::dc_arrow;
    else if (cursor_name == "text")
        cursor_code = vgui::CursorCode::dc_ibeam;
    else if (cursor_name == "pointer")
        cursor_code = vgui::CursorCode::dc_hand;
    else if (cursor_name == "cross")
        cursor_code = vgui::CursorCode::dc_crosshair;
    else if (cursor_name == "wait")
        cursor_code = vgui::CursorCode::dc_hourglass;
    else if (cursor_name == "move")
        cursor_code = vgui::CursorCode::dc_sizeall;
    else if (cursor_name == "resize-nwse")
        cursor_code = vgui::CursorCode::dc_sizenwse;
    else if (cursor_name == "resize-nesw")
        cursor_code = vgui::CursorCode::dc_sizenesw;
    else if (cursor_name == "resize-we")
        cursor_code = vgui::CursorCode::dc_sizewe;
    else if (cursor_name == "resize-ns")
        cursor_code = vgui::CursorCode::dc_sizens;
    else if (cursor_name == "not-allowed" || cursor_name == "no-drop")
        cursor_code = vgui::CursorCode::dc_no;
    else if (cursor_name == "none")
        cursor_code = vgui::CursorCode::dc_none;
    else if (cursor_name == "auto")
        cursor_code = vgui::CursorCode::dc_blank; // Default to system cursor

    vgui::input()->SetCursorOveride(cursor_code);
}

int RmlUiSystemInterface::TranslateString(Rml::String& translated, const Rml::String& input)
{
    if (input[0] == '#')
    {

        std::wstring ws;
        wchar_t* szText = g_pVGuiLocalize->Find(input.c_str());

        if (!szText)
        {
            SystemInterface::TranslateString(translated, input);
            return 0;
        }

        ws = szText;

        // Convert wstring to string using some C++11 magic
        using convert_type = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_type, wchar_t> converter;

        std::string localizedString = converter.to_bytes(ws);

        if (!localizedString.empty())
        {
            // Sorta hack, call base class TranslateString with input of localized string
            SystemInterface::TranslateString(translated, localizedString);
            return 1;
        }
    }

    // Sorta hack, setting translatied to input causes heap corruption (C++ in nutshell)
    SystemInterface::TranslateString(translated, input);
    return 0;
}