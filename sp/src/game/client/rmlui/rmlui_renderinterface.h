// ==================================================================
// Den Urakolouy AKA URAKOLOUY5
// 2025
// 
// Feel free to use it as you want to use.
// Major code based on open source references from Source SDK.
// ==================================================================

#ifndef RMLUI_RENDERINTERFACE
#define RMLUI_RENDERINTERFACE

#ifdef Assert
#undef Assert
#endif

#include <RmlUi/Core/Core.h>
#include "RmlUi/Core/RenderInterface.h"
#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/DecorationTypes.h>

#ifdef DBGFLAG_ASSERT
#define  Assert( _exp )           							_AssertMsg( _exp, _T("Assertion Failed: ") _T(#_exp), ((void)0), false )
#else
#define  Assert( _exp )										((void)0)
#endif

#include "materialsystem/itexture.h"
#include "materialsystem/imaterialvar.h"
#include "pixelwriter.h"
#include "vtf/vtf.h"
#include "tier1/utldict.h"

class RmlUIRenderInterface : public Rml::RenderInterface {
private:
	CMaterialReference m_pGeometryMaterial;
	CMaterialReference m_pWireframeMaterial;
	VMatrix transform;
	bool transformEnabled;
	int stencilTestValue;
	CMatRenderContextPtr pRenderContext;

public:
	// ---------------------------- Our methods ----------------------------

	void BeginFrame();
	void EndFrame();

	// ------------------------------ Geometry -----------------------------

	/// Called by RmlUi when it wants to compile geometry to be rendered later.
	Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;

	/// Called by RmlUi when it wants to render geometry.
	/// It creates mesh, renders it and destroys it 
	/// each call
	void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;

	/// Called by RmlUi when it wants to release geometry.
	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;
	
	// ------------------------------ Textures -----------------------------

	/// Called by RmlUi when a texture is required to be generated from a sequence of pixels in memory (for example, fonts)
	Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;
	
	/// Called by RmlUi when a texture is required by the library.
	Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;

	/// Called by RmlUi when a loaded or generated texture is no longer required.
	void ReleaseTexture(Rml::TextureHandle texture) override;
	
	// ----------------------- Clipping & Transforms -----------------------

	/// Called by RmlUi when it wants to enable or disable scissoring to clip content.
	void EnableScissorRegion(bool enable) override;

	/// Called by RmlUi when it wants to change the scissor region.
	void SetScissorRegion(Rml::Rectanglei region) override;

	/// Called by RmlUi when it wants the renderer to use a new transform matrix.
	void SetTransform(const Rml::Matrix4f* newTransform) override;

	/// Called by RmlUi when it wants to enable or disable the clip mask.
	void EnableClipMask(bool enable) override;

	/// Called by RmlUi when it wants to set or modify the contents of the clip mask.
	void RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation) override;

	// ------------------------------ Shaders ------------------------------
	
	/// Called by RmlUi when it wants to compile a new shader.
	Rml::CompiledShaderHandle CompileShader(const Rml::String& name, const Rml::Dictionary& parameters) override;

	/// Called by RmlUi when it wants to render created shader.
	void RenderShader(Rml::CompiledShaderHandle shader, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;

	/// Called by RmlUi when it wants to release (destroy) created shader.
	void ReleaseShader(Rml::CompiledShaderHandle shader) override;
};
#endif

/// Custom render interface to connect RmlUi with engine's material system
class RmlUIProceduralRegenerator : public ITextureRegenerator
{
private:
	std::vector<Rml::byte> dataBuffer;

public:
	RmlUIProceduralRegenerator(void) {}

	/// Generate texture
	virtual void RegenerateTextureBits(ITexture* pTexture, IVTFTexture* pVTFTexture, Rect_t* pSubRect) override;

	/// Release (nothing to remove)
	virtual void Release(void) override;

	/// Set data
	void SetData(const Rml::byte* _data, size_t dataSize);
};