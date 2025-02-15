// ==================================================================
// Den Urakolouy AKA URAKOLOUY5
// 2025
// 
// Feel free to use it as you want to use.
// Major code based on open source references from Source SDK.
// ==================================================================

#include "cbase.h"

#include "rmlui_manager.h"
#include "rmlui_renderinterface.h"
#include "rmlui_systeminterface.h"
#include "rmlui_filesysteminterface.h"
#include "filesystem.h"
#include "ienginevgui.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "GameUI/IGameUI.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

RmlUIManager* RmlUIManager::instance = nullptr;
RmlUIManager::RmlUIManager() {}

static RmlUIRenderInterface renderInterface;
static RmlUiSystemInterface systemInterface;
static RmlUiFileInterface fileInterface;

// See interface.h/.cpp for specifics:  basically this ensures that we actually Sys_UnloadModule the dll and that we don't call Sys_LoadModule 
//  over and over again.
static CDllDemandLoader g_GameUI("GameUI");

/// Create or get singleton instance of this manager
RmlUIManager* RmlUIManager::GetInstance()
{
	if (instance == nullptr)
		instance = new RmlUIManager();

	return instance;
}

/// Initialize RmlUi and assets
void RmlUIManager::Init()
{
	// Set our interfaces to work with Source Engine
	Rml::SetRenderInterface(&renderInterface);
	Rml::SetSystemInterface(&systemInterface);
	Rml::SetFileInterface(&fileInterface);

	// Initialize RmlUi
	Rml::Initialise();

	// Load all fonts in mod's resource folder
	LoadFontFaces();

	// Create VGUI panel for main menuat
	// (panel used also to detect mouse input and other stuff)
	// TODO: If there's better way than using VGUI to rely on input system
	// let me know
	rmlPanel = new RmlUiPanel();

	CreateInterfaceFn gameUIFactory = g_GameUI.GetFactory();
	if (gameUIFactory)
	{
		IGameUI* m_pGameUI = (IGameUI*)gameUIFactory(GAMEUI_INTERFACE_VERSION, NULL);

		m_pGameUI->SetMainMenuOverride(rmlPanel->GetVPanel());
	}

	CreateContext("main", "rmlui/mainmenu.rml");
	CreateContext("hud", "rmlui/hud.rml");
}

/// Render all contexts
void RmlUIManager::Render(const char* contextName)
{
	Rml::Context* context = contexts[contextName];

	if (context)
	{
		CMatRenderContextPtr pRenderContext(materials);

		// Disable depth write
		pRenderContext->OverrideDepthEnable(true, false);

		// Ortho matrix
		pRenderContext->MatrixMode(MATERIAL_MODEL);
		pRenderContext->PushMatrix();
		pRenderContext->LoadIdentity();

		pRenderContext->MatrixMode(MATERIAL_PROJECTION);
		pRenderContext->PushMatrix();

		// Disable previous transform
		renderInterface.DisableTransform();

		// Render contexts
		context->Update();
		context->Render();

		// Restore matrices
		pRenderContext->MatrixMode(MATERIAL_MODEL);
		pRenderContext->PopMatrix();

		pRenderContext->MatrixMode(MATERIAL_PROJECTION);
		pRenderContext->PopMatrix();

		// Restore override write to depth
		pRenderContext->OverrideDepthEnable(false, false);
	}
}

/// Called when resolution changes
void RmlUIManager::OnScreenSizeChanged(int iOldWide, int iOldTall)
{
	//pContext->SetDimensions(Rml::Vector2i(ScreenWidth(), ScreenHeight()));
}

static Rml::Input::KeyIdentifier ConvertKeyCodeTo(ButtonCode_t keynum)
{
	switch (keynum)
	{
		case KEY_0: return Rml::Input::KI_0;
		case KEY_1: return Rml::Input::KI_1;
		case KEY_2: return Rml::Input::KI_2;
		case KEY_3: return Rml::Input::KI_3;
		case KEY_4: return Rml::Input::KI_4;
		case KEY_5: return Rml::Input::KI_5;
		case KEY_6: return Rml::Input::KI_6;
		case KEY_7: return Rml::Input::KI_7;
		case KEY_8: return Rml::Input::KI_8;
		case KEY_9: return Rml::Input::KI_9;
		case KEY_A: return Rml::Input::KI_A;
		case KEY_B: return Rml::Input::KI_B;
		case KEY_C: return Rml::Input::KI_C;
		case KEY_D: return Rml::Input::KI_D;
		case KEY_E: return Rml::Input::KI_E;
		case KEY_F: return Rml::Input::KI_F;
		case KEY_G: return Rml::Input::KI_G;
		case KEY_H: return Rml::Input::KI_H;
		case KEY_I: return Rml::Input::KI_I;
		case KEY_J: return Rml::Input::KI_J;
		case KEY_K: return Rml::Input::KI_K;
		case KEY_L: return Rml::Input::KI_L;
		case KEY_M: return Rml::Input::KI_M;
		case KEY_N: return Rml::Input::KI_N;
		case KEY_O: return Rml::Input::KI_O;
		case KEY_P: return Rml::Input::KI_P;
		case KEY_Q: return Rml::Input::KI_Q;
		case KEY_R: return Rml::Input::KI_R;
		case KEY_S: return Rml::Input::KI_S;
		case KEY_T: return Rml::Input::KI_T;
		case KEY_U: return Rml::Input::KI_U;
		case KEY_V: return Rml::Input::KI_V;
		case KEY_W: return Rml::Input::KI_W;
		case KEY_X: return Rml::Input::KI_X;
		case KEY_Y: return Rml::Input::KI_Y;
		case KEY_Z: return Rml::Input::KI_Z;

		case KEY_PAD_0: return Rml::Input::KI_NUMPAD0;
		case KEY_PAD_1: return Rml::Input::KI_NUMPAD1;
		case KEY_PAD_2: return Rml::Input::KI_NUMPAD2;
		case KEY_PAD_3: return Rml::Input::KI_NUMPAD3;
		case KEY_PAD_4: return Rml::Input::KI_NUMPAD4;
		case KEY_PAD_5: return Rml::Input::KI_NUMPAD5;
		case KEY_PAD_6: return Rml::Input::KI_NUMPAD6;
		case KEY_PAD_7: return Rml::Input::KI_NUMPAD7;
		case KEY_PAD_8: return Rml::Input::KI_NUMPAD8;
		case KEY_PAD_9: return Rml::Input::KI_NUMPAD9;

		case KEY_ENTER: return Rml::Input::KI_RETURN;
		case KEY_SPACE: return Rml::Input::KI_SPACE;
		case KEY_BACKSPACE: return Rml::Input::KI_BACK;
		case KEY_TAB: return Rml::Input::KI_TAB;
		case KEY_CAPSLOCK: return Rml::Input::KI_CAPITAL;
		case KEY_ESCAPE: return Rml::Input::KI_ESCAPE;

		case KEY_UP: return Rml::Input::KI_UP;
		case KEY_LEFT: return Rml::Input::KI_LEFT;
		case KEY_DOWN: return Rml::Input::KI_DOWN;
		case KEY_RIGHT: return Rml::Input::KI_RIGHT;

		// For keys that don't directly map, we can either skip or handle them as custom
		case BUTTON_CODE_INVALID:
		case BUTTON_CODE_NONE:
		default: return Rml::Input::KI_UNKNOWN;
	}
}

/// Based on vgui2/vgui_controls/HTML.cpp
static int GetKeyModifiers()
{
	// Any time a key is pressed reset modifier list as well
	int modifierState = 0;

	if (vgui::input()->IsKeyDown(KEY_LCONTROL) || vgui::input()->IsKeyDown(KEY_RCONTROL))
		modifierState |= Rml::Input::KeyModifier::KM_CTRL;

	if (vgui::input()->IsKeyDown(KEY_LALT) || vgui::input()->IsKeyDown(KEY_RALT))
		modifierState |= Rml::Input::KeyModifier::KM_ALT;

	if (vgui::input()->IsKeyDown(KEY_LSHIFT) || vgui::input()->IsKeyDown(KEY_RSHIFT))
		modifierState |= Rml::Input::KeyModifier::KM_SHIFT;

	if (vgui::input()->IsKeyDown(KEY_CAPSLOCK))
		modifierState |= Rml::Input::KeyModifier::KM_CAPSLOCK;

	if (vgui::input()->IsKeyDown(KEY_NUMLOCK))
		modifierState |= Rml::Input::KeyModifier::KM_NUMLOCK;

	if (vgui::input()->IsKeyDown(KEY_LWIN) && vgui::input()->IsKeyDown(KEY_RWIN))
		modifierState |= Rml::Input::KeyModifier::KM_META;

	if (vgui::input()->IsKeyDown(KEY_SCROLLLOCK))
		modifierState |= Rml::Input::KeyModifier::KM_SCROLLLOCK;

	return modifierState;
}
	
static int GetMouseButtonIndex(vgui::MouseCode code)
{
	switch (code)
	{
		case (MOUSE_LEFT):
			return 0;
		case (MOUSE_RIGHT):
			return 1;
		case (MOUSE_MIDDLE):
			return 2;
		default:
			return -1;
	}
}

/// Called when key code pressed
void RmlUIManager::OnKeyCodePressed(ButtonCode_t keynum)
{
	//pContext->ProcessKeyDown(ConvertKeyCodeTo(keynum), GetKeyModifiers());
}

/// Called when key code released
void RmlUIManager::OnKeyCodeReleased(ButtonCode_t keynum)
{
	//pContext->ProcessKeyUp(ConvertKeyCodeTo(keynum), GetKeyModifiers());
}

void RmlUIManager::OnKeyTyped(wchar_t unichar)
{
	//if (unichar != 8)
		//pContext->ProcessTextInput(static_cast<Rml::Character>(unichar));
}

void RmlUIManager::SetInputEnabled(bool state)
{
	rmlPanel->SetMouseInputEnabled(state);
	rmlPanel->SetKeyBoardInputEnabled(state);
}

void RmlUIManager::OnCursorMoved(int x, int y)
{
	//Msg("OnCursorMoved X: %i, Y: %i", x, y);

	//bool mouseMove = pContext->ProcessMouseMove(x, y, GetKeyModifiers());

	//Msg("mouseMoveHit == %s\n", mouseMove ? "true" : "false");
}

void RmlUIManager::OnMousePressed(vgui::MouseCode code)
{
	Msg("%i %i \n", GetMouseButtonIndex(code), code);
	//pContext->ProcessMouseButtonDown(GetMouseButtonIndex(code), GetKeyModifiers());
}

void RmlUIManager::OnMouseDoublePressed(vgui::MouseCode code)
{
}

void RmlUIManager::OnMouseReleased(vgui::MouseCode code)
{
	//pContext->ProcessMouseButtonUp(GetMouseButtonIndex(code), GetKeyModifiers());
}

void RmlUIManager::OnMouseWheeled(int delta)
{
	// Apply vertical delta
	// Invert delta to scroll properly
	//pContext->ProcessMouseWheel(Rml::Vector2f(0, -delta), GetKeyModifiers());
}

/// Shutdown
void RmlUIManager::Shutdown()
{
	delete rmlPanel;
	rmlPanel = nullptr;
	Rml::Shutdown();
}

/// Load the document and attach it to specified vgui panel
/// and create new context for it
Rml::Context* RmlUIManager::CreateContext(const char* contextName, Rml::String documentName)
{
	if (contexts[contextName])
	{
		Rml::Log::Message(Rml::Log::LT_ERROR, "Context with name %s already exists", contextName);
		return nullptr;
	}

	Rml::Context* newContext = Rml::CreateContext(contextName, Rml::Vector2i(ScreenWidth(), ScreenHeight()));
	Rml::ElementDocument* document = newContext->LoadDocument(documentName);
	document->Show();

	contexts[contextName] = newContext;

	return newContext;
}

/// Retrieve context by name if found
Rml::Context* RmlUIManager::GetContext(const char* contextName)
{
	if (contexts[contextName])
	{
		Rml::Context* context = contexts[contextName];
		return context;
	}

	return nullptr;
}

/// Load all font files in mod
void RmlUIManager::LoadFontFaces()
{
	FileFindHandle_t findHandle;
	const char* fileName = g_pFullFileSystem->FindFirstEx("resource/*.ttf", "MOD", &findHandle);

	while (fileName)
	{
		char fullPath[MAX_PATH];
		V_snprintf(fullPath, sizeof(fullPath), "resource/%s", fileName);

		Rml::LoadFontFace(fullPath);

		fileName = g_pFullFileSystem->FindNext(findHandle);
	}
}

RmlUIManager::~RmlUIManager() {
	Shutdown();
}

void Cmd_RmlUiDebugger(const CCommand& args)
{
	Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
	RmlUIManager::GetInstance()->SetInputEnabled(!Rml::Debugger::IsVisible());
}

ConCommand rmlui_debugger("rmlui_debugger", Cmd_RmlUiDebugger, "Shows debug plugin (inspector) for RmlUi", FCVAR_NONE);