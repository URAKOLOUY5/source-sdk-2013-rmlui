// ==================================================================
// Den Urakolouy AKA URAKOLOUY5
// 2025
// 
// Feel free to use it as you want to use.
// Major code based on open source references from Source SDK.
// ==================================================================

#ifndef RMLUI_SYSTEMINTERFACE
#define RMLUI_SYSTEMINTERFACE

#ifdef Assert
#undef Assert
#endif

#include "RmlUi/Core/SystemInterface.h"

#ifdef DBGFLAG_ASSERT
#define  Assert( _exp )           							_AssertMsg( _exp, _T("Assertion Failed: ") _T(#_exp), ((void)0), false )
#else
#define  Assert( _exp )										((void)0)
#endif

/// Custom system interface to connect RmlUi and Source SDK system calls
class RmlUiSystemInterface : public Rml::SystemInterface {
	bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;
	void JoinPath(Rml::String& translated_path, const Rml::String& document_path, const Rml::String& path) override;
	void SetMouseCursor(const Rml::String& cursor_name) override;
};

#endif