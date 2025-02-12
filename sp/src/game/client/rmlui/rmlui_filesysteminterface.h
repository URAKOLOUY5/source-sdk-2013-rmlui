// ==================================================================
// Den Urakolouy AKA URAKOLOUY5
// 2025
// 
// Feel free to use it as you want to use.
// Major code based on open source references from Source SDK.
// ==================================================================

#ifndef RMLUI_FILEINTERFACE
#define RMLUI_FILEINTERFACE

#ifdef Assert
#undef Assert
#endif

#include "RmlUi/Core/FileInterface.h"

#ifdef DBGFLAG_ASSERT
#define  Assert( _exp )           							_AssertMsg( _exp, _T("Assertion Failed: ") _T(#_exp), ((void)0), false )
#else
#define  Assert( _exp )										((void)0)
#endif

class RmlUiFileInterface : public Rml::FileInterface {
	// Inherited via FileInterface
	Rml::FileHandle Open(const Rml::String& path) override;
	void Close(Rml::FileHandle file) override;
	size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;
	bool Seek(Rml::FileHandle file, long offset, int origin) override;
	size_t Tell(Rml::FileHandle file) override;
};

#endif