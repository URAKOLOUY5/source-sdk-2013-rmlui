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
#include <rmlui/Core.h>
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

class RmlRenderLayerStack {
public:
	RmlRenderLayerStack();
	~RmlRenderLayerStack();

	Rml::LayerHandle PushLayer();
	void PopLayer();
	ITexture* GetLayer(Rml::LayerHandle layer) const;
	ITexture* GetTopLayer() const;
	Rml::LayerHandle GetTopLayerHandle() const;

	ITexture* GetPostprocessPrimary() { return EnsureFramebufferPostprocess(0); }
	ITexture* GetPostprocessSecondary() { return EnsureFramebufferPostprocess(1); }
	ITexture* GetPostprocessTertiary() { return EnsureFramebufferPostprocess(2); }
	ITexture* GetBlendMask() { return EnsureFramebufferPostprocess(3); }

	void SwapPostprocessPrimarySecondary();

	void BeginFrame(int new_width, int new_height);
	void EndFrame();

private:
	void DestroyFramebuffers();
	ITexture* EnsureFramebufferPostprocess(int index);
	int width = 0, height = 0;
	
	// The number of active layers is manually tracked since we re-use the framebuffers stored in the fb_layers stack.
	int layersSize = 0;

	Rml::Vector<ITexture*> fbLayers;
	Rml::Vector<ITexture*> fbPostProcess;

};

class RmlUIRenderInterface : public Rml::RenderInterface {
private:
	CMaterialReference m_pGeometryMaterial;
	CMaterialReference m_pWireframeMaterial;
	VMatrix transform;
	bool transformEnabled;
	int stencilTestValue;
	RmlRenderLayerStack renderLayers;

	void BlitLayerToPostprocessPrimary(Rml::LayerHandle layer_handle);
	void RenderFilters(Rml::Span<const Rml::CompiledFilterHandle> filter_handles);

public:

	// Inherited via RenderInterface
	void BeginFrame();
	void EndFrame();
	Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) override;
	void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
	void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;
	Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
	Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions) override;
	void ReleaseTexture(Rml::TextureHandle texture) override;
	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(Rml::Rectanglei region) override;
	void SetTransform(const Rml::Matrix4f* newTransform) override;
	void EnableClipMask(bool enable) override;
	void RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation) override;
	Rml::CompiledShaderHandle CompileShader(const Rml::String& name, const Rml::Dictionary& parameters) override;
	void RenderShader(Rml::CompiledShaderHandle shader, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture) override;
	void ReleaseShader(Rml::CompiledShaderHandle shader) override;
	void DisableTransform();
	Rml::LayerHandle PushLayer() override;
	void PopLayer() override;
	void CompositeLayers(Rml::LayerHandle source,
		Rml::LayerHandle destination,
		Rml::BlendMode blend_mode,
		Rml::Span<const Rml::CompiledFilterHandle> filters) override;
	Rml::CompiledFilterHandle CompileFilter(const Rml::String& name, const Rml::Dictionary& parameters) override;
	void ReleaseFilter(Rml::CompiledFilterHandle filter) override;
};
#endif

/// Custom render interface to connect RmlUi with engine's material system
class RmlUIProceduralRegenerator : public ITextureRegenerator
{
private:
	std::vector<Rml::byte> dataBuffer;

public:
	RmlUIProceduralRegenerator(void) {}
	virtual void RegenerateTextureBits(ITexture* pTexture, IVTFTexture* pVTFTexture, Rect_t* pSubRect);
	virtual void Release(void);

	void SetData(const Rml::byte* _data, size_t dataSize);
};