// ==================================================================
// Den Urakolouy AKA URAKOLOUY5
// 2025
// 
// Feel free to use it as you want to use.
// Major code based on open source references from Source SDK.
// ==================================================================

#include "cbase.h"

#include "rmlui_renderinterface.h"
#include "bitmap/tgaloader.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rmlui_wireframe("rmlui_wireframe", "0", FCVAR_CHEAT, "Render wireframe instead of geometry");

#define MAX_NUM_STOPS 16

#define GRADIENT_LINEAR             0
#define GRADIENT_RADIAL             1
#define GRADIENT_CONIC              2
#define GRADIENT_REPEATING_LINEAR   3
#define GRADIENT_REPEATING_RADIAL   4
#define GRADIENT_REPEATING_CONIC    5

class GeometryHandle
{
public:
	Rml::Span<const Rml::Vertex> vertices;
	Rml::Span<const int> indices;
};

class TextureHandle
{
public:
    CTextureReference texture;
    CMaterialReference material;
};

// Functions for gradient shader (match parameters of rmlui_gradient pixel shader)
enum class ShaderGradientFunction { Linear, Radial, Conic, RepeatingLinear, RepeatingRadial, RepeatingConic };

// Type of compiled shader geometry
// There's room for custom decorator shaders
enum class CompiledShaderType { Invalid = 0, Gradient };

/// Called by RmlUi when it wants to compile geometry to be rendered later.
Rml::CompiledGeometryHandle RmlUIRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices)
{
    auto* handle = new GeometryHandle();
    handle->vertices = vertices;
    handle->indices = indices;

    return reinterpret_cast<Rml::CompiledGeometryHandle>(handle);
}

/// Called by RmlUi when it wants to render geometry.
/// It creates mesh, renders it and destroys it 
/// each call
void RmlUIRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle texture)
{
    auto* handle = reinterpret_cast<GeometryHandle*>(geometry);
    if (!handle)
        return;

    // Setup fallback material for geometry
    if (!m_pGeometryMaterial)
    {
        KeyValues* pVMTKeyValues = new KeyValues("UnlitGeneric");
        pVMTKeyValues->SetInt("$vertexcolor", 1); // Use color from vertex
        pVMTKeyValues->SetInt("$vertexalpha", 1); // Use alpha from vertex
        pVMTKeyValues->SetInt("$ignorez", 1); // Ignore depth
        pVMTKeyValues->SetInt("$translucent", 1); // Enable transparency
        pVMTKeyValues->SetInt("$no_fullbright", 1); // Ignore fullbright
        IMaterial* mat = g_pMaterialSystem->FindProceduralMaterial("__rml_geometry", TEXTURE_GROUP_OTHER, pVMTKeyValues);
        m_pGeometryMaterial.Init(mat);
    }

    // Setup wireframe material for geometry
    if (!m_pWireframeMaterial)
    {
        IMaterial* wireframeRef = materials->FindMaterial("debug/debugwireframevertexcolor", TEXTURE_GROUP_OTHER);
        m_pWireframeMaterial.Init(wireframeRef);
    }

    CMatRenderContextPtr pRenderContext(materials);

    pRenderContext->MatrixMode(MATERIAL_MODEL);
    pRenderContext->PushMatrix();

    // Translate matrix
    pRenderContext->Translate(translation.x, translation.y, 0.0f);

    if (transformEnabled)
        pRenderContext->MultMatrix(transform);

    // Extract material from TextureHandle instance and bind it
    TextureHandle* textureHandle = reinterpret_cast<TextureHandle*>(texture);

    // Bind texture from handle or fallback to geo material
    if (!rmlui_wireframe.GetBool())
    {
        if (textureHandle)
            pRenderContext->Bind(textureHandle->material);
        else
            pRenderContext->Bind(m_pGeometryMaterial);
    }
    else
    {
        pRenderContext->Bind(m_pWireframeMaterial);
    }

    // Build the mesh
    IMesh* pMesh = pRenderContext->GetDynamicMesh(true);
    if (!pMesh)
        return;

    CMeshBuilder meshBuilder;
    meshBuilder.Begin(pMesh, MATERIAL_TRIANGLES, handle->vertices.size(), handle->indices.size());

    // In RmlUi mesh consists of triangles (3 vertices per triangle)
    //  Each vertex position is a pixel coordinate
    //  The coordinate system of documents in RmlUi places the origin at the top-left corner of the window (same for DirectX).
    //  The generated textures in RmlUi use the convention with the origin placed at the bottom-left corner. (haven't checked, it works btw)
    for (const auto& vertex : handle->vertices)
    {
        meshBuilder.Position3f(vertex.position.x, vertex.position.y, 0);
        meshBuilder.Color4ub
        (
            vertex.colour.red * vertex.colour.alpha / 255,
            vertex.colour.green * vertex.colour.alpha / 255,
            vertex.colour.blue * vertex.colour.alpha / 255,
            vertex.colour.alpha
        );
        meshBuilder.TexCoord2f(0, vertex.tex_coord.x, vertex.tex_coord.y); // UV
        meshBuilder.AdvanceVertexF<VTX_HAVEPOS | VTX_HAVECOLOR, 1>();
    }

    // Swap indexes to invert mesh
    // so we can see front face instead of backface
    for (size_t i = 0; i < handle->indices.size(); i += 3)
    {
        meshBuilder.FastIndex(handle->indices[i]);
        meshBuilder.FastIndex(handle->indices[i + 2]);
        meshBuilder.FastIndex(handle->indices[i + 1]);
    }

    meshBuilder.End();

    // We're done, draw the mesh now
    pMesh->Draw();

    pRenderContext->PopMatrix();
}

/// Called by RmlUi when it wants to release geometry.
void RmlUIRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
    auto* handle = reinterpret_cast<GeometryHandle*>(geometry);

    if (handle)
        delete handle;
}

/// Called by RmlUi when a texture is required to be generated from a sequence of pixels in memory (for example, fonts)
Rml::TextureHandle RmlUIRenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source, Rml::Vector2i source_dimensions)
{
    TextureHandle* handle = new TextureHandle();

    // We don't need empty stuff
    if (source.empty())
        return Rml::TextureHandle();

    // Cool hash name out of source
    uint32_t hash = CRC32_ProcessSingleBuffer(source.data(), source.size());
    char pName[16];
    Q_snprintf(pName, 16, "__rml_%08X%2", hash);

    // Setup texture
    ITexture* pTexture = materials->CreateProceduralTexture(
        pName,
        TEXTURE_GROUP_VGUI,
        source_dimensions.x,
        source_dimensions.y,
        IMAGE_FORMAT_RGBA8888,
        TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_NOMIP |
        TEXTUREFLAGS_PROCEDURAL | TEXTUREFLAGS_SINGLECOPY
    );
    
    // Create our procedular texture generator
    // to draw onto texture with it
    RmlUIProceduralRegenerator* generator = new RmlUIProceduralRegenerator();
    generator->SetData(source.data(), source.size());
    pTexture->SetTextureRegenerator(generator);
    
    handle->texture.Init(pTexture);
    pTexture->Download(); // Generate texture now
  
    // Setup material to bind it later
    KeyValues* pVMTKeyValues = new KeyValues("UnlitGeneric");
    pVMTKeyValues->SetString("$basetexture", pName);
    pVMTKeyValues->SetInt("$vertexcolor", 1); // Use color from vertex
    pVMTKeyValues->SetInt("$vertexalpha", 1); // Use alpha from vertex
    pVMTKeyValues->SetInt("$ignorez", 1); // Ignore depth
    pVMTKeyValues->SetInt("$translucent", 1); // Enable transparency
    pVMTKeyValues->SetInt("$no_fullbright", 1); // Ignore fullbright
    IMaterial* pMaterial = materials->CreateMaterial(pName, pVMTKeyValues);

    handle->material.Init(pMaterial);

    return reinterpret_cast<Rml::TextureHandle>(handle);
}

/// Called by RmlUi when a texture is required by the library.
Rml::TextureHandle RmlUIRenderInterface::LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source)
{
    // Allocate a CUtlMemory to hold the RGBA8888 image data.
    CUtlMemory<unsigned char> outputData;
    int outWidth = 0, outHeight = 0;

    // Load the TGA image data in RGBA8888 format.
    // TGALoader::LoadRGBA8888 will fill outputData and return true on success.
    if (!TGALoader::LoadRGBA8888(source.c_str(), outputData, outWidth, outHeight))
    {
        // If the TGA file could not be loaded, return an empty texture handle.
        return Rml::TextureHandle();
    }

    // Update the texture dimensions based on the loaded image.
    texture_dimensions.x = outWidth;
    texture_dimensions.y = outHeight;

    // Compute the total number of bytes in the image.
    // We expect 4 bytes per pixel (RGBA8888).
    size_t dataSize = static_cast<size_t>(outWidth * outHeight * 4);

    // Create a span from the loaded image data.
    // Rml::byte is assumed to be equivalent to unsigned char.
    Rml::Span<const Rml::byte> imageDataSpan(reinterpret_cast<Rml::byte*>(outputData.Base()), dataSize);

    // Call your existing GenerateTexture function, which creates a procedural texture from the image data.
    return GenerateTexture(imageDataSpan, Rml::Vector2i(outWidth, outHeight));
}

/// Called by RmlUi when a loaded or generated texture is no longer required.
void RmlUIRenderInterface::ReleaseTexture(Rml::TextureHandle texture)
{
    TextureHandle* handle = reinterpret_cast<TextureHandle*>(texture);

    if (handle)
    {
        CMaterialReference materialRef = handle->material;
        CTextureReference textureRef = handle->texture;

        if (textureRef)
        {
            textureRef->SetTextureRegenerator(NULL);
            textureRef.Shutdown();
        }

        if (materialRef)
            materialRef.Shutdown();

        delete handle;
    }
}

/// Called by RmlUi when it wants to enable or disable scissoring to clip content.
void RmlUIRenderInterface::EnableScissorRegion(bool enable)
{
	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->SetScissorRect(0, 0, ScreenWidth(), ScreenHeight(), enable);

}

/// Called by RmlUi when it wants to change the scissor region.
void RmlUIRenderInterface::SetScissorRegion(Rml::Rectanglei region)
{
	CMatRenderContextPtr pRenderContext(materials);
    pRenderContext->SetScissorRect(region.Left(), region.Top(), region.Right(), region.Bottom(), true);
}

/// Called by RmlUi when it wants the renderer to use a new transform matrix.
void RmlUIRenderInterface::SetTransform(const Rml::Matrix4f* newTransform)
{
    CMatRenderContextPtr pRenderContext(materials);
    transformEnabled = (newTransform != nullptr);

    if (newTransform)
    {
        const float* src = newTransform->data();

        // Initialize the VMatrix using the 16 elements
        // as row-major matrix
        transform = VMatrix(
            src[0], src[4], src[8],  src[12],
            src[1], src[5], src[9],  src[13],
            src[2], src[6], src[10], src[14],
            src[3], src[7], src[11], src[15]
        );
    }
}

/// Called by RmlUi when it wants to enable or disable the clip mask.
void RmlUIRenderInterface::EnableClipMask(bool enable)
{
    CMatRenderContextPtr pRenderContext(materials);
    pRenderContext->SetStencilEnable(enable);
}

/// Called by RmlUi when it wants to set or modify the contents of the clip mask.
/// Reference: https://github.com/mikke89/RmlUi/blob/b48abfbd5fc333d9a5c201c1a4b2a3f5881308e8/Backends/RmlUi_Renderer_GL2.cpp#L163
void RmlUIRenderInterface::RenderToClipMask(Rml::ClipMaskOperation operation, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation)
{
    auto* handle = reinterpret_cast<GeometryHandle*>(geometry);
    if (!handle)
        return;

    CMatRenderContextPtr pRenderContext(materials);

    // Disable color writes so that only the stencil buffer is modified.
    pRenderContext->OverrideColorWriteEnable(true, false);

    // Set up the stencil function to always pass.
    pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_ALWAYS);

    // We initially set the reference to 1—this may be adjusted below.
    pRenderContext->SetStencilReferenceValue(1);
    pRenderContext->SetStencilWriteMask(0xFF);

    switch (operation)
    {
        case Rml::ClipMaskOperation::Set:
        {
            // Render the mask geometry and replace the stencil value with 1.
            pRenderContext->SetStencilPassOperation(STENCILOPERATION_REPLACE);
            stencilTestValue = 1;
            break;
        }
        case Rml::ClipMaskOperation::SetInverse:
        {
            // For an inverted mask, render and replace the stencil value with 0.
            pRenderContext->SetStencilPassOperation(STENCILOPERATION_REPLACE);
            stencilTestValue = 0;
            break;
        }
        case Rml::ClipMaskOperation::Intersect:
        {
            // For an intersection, increment the stencil where geometry overlaps.
            pRenderContext->SetStencilPassOperation(STENCILOPERATION_INCR);
            stencilTestValue += 1;
            break;
        }
    }

    // Render our geometry mask (pass empty material handle, so it fallback to geometry material)
    RenderGeometry(geometry, translation, Rml::TextureHandle());

    // Restore color write
    pRenderContext->OverrideColorWriteEnable(false, false);

    // Now set the stencil state to allow only pixels with our target value to be rendered.
    pRenderContext->SetStencilPassOperation(STENCILOPERATION_KEEP);
    pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_EQUAL);
    pRenderContext->SetStencilReferenceValue(stencilTestValue);
}

/// Called by RmlUi when it wants to compile a new shader.
Rml::CompiledShaderHandle RmlUIRenderInterface::CompileShader(const Rml::String& name, const Rml::Dictionary& parameters)
{
    TextureHandle* shaderHandle = new TextureHandle();
    CMaterialReference material;

    // Create VMT params (with default Unlit shader)
    KeyValues* pVMTKeyValues = new KeyValues("UnlitGeneric");
    char pMaterialName[255];
    
    // Setup shader name
    if (name == "linear-gradient" || name == "radial-gradient" || name == "conic-gradient")
    {
        pVMTKeyValues->SetName("RmlUi_Gradient");

        const bool repeating = Rml::Get(parameters, "repeating", false);

        if (name == "linear-gradient")
            pVMTKeyValues->SetInt("$FUNC", repeating ? GRADIENT_REPEATING_LINEAR : GRADIENT_LINEAR);
        else if (name == "radial-gradient")
            pVMTKeyValues->SetInt("$FUNC", repeating ? GRADIENT_REPEATING_RADIAL : GRADIENT_RADIAL);
        else if (name == "conic-gradient")
            pVMTKeyValues->SetInt("$FUNC", repeating ? GRADIENT_REPEATING_CONIC : GRADIENT_CONIC);

        const Rml::ColorStopList colorStopList = Rml::Get(parameters, "color_stop_list", Rml::ColorStopList());

        char p[32];
        char v[32];

        Rml::Vector2f p0;
        Rml::Vector2f p1;

        if (name == "linear-gradient")
        {
            p0 = Rml::Get(parameters, "p0", Rml::Vector2f(0.f));
            p1 = Rml::Get(parameters, "p1", Rml::Vector2f(0.f)) - p0;
        }
        else if (name == "radial-gradient")
        {
            p0 = Rml::Get(parameters, "center", Rml::Vector2f(0.f));
            p1 = Rml::Get(parameters, "radius", Rml::Vector2f(0.5f));
        }
        else if (name == "conic-gradient")
        {
            p0 = Rml::Get(parameters, "center", Rml::Vector2f(0.f));
            float angle = Rml::Get(parameters, "angle", 0.f);
            p1 = Rml::Vector2f(Rml::Math::Cos(angle), Rml::Math::Sin(angle));
        }

        // Cool unique name for shader based material
        uint32_t hash = CRC32_ProcessSingleBuffer(colorStopList.data(), colorStopList.size());
        Q_snprintf(pMaterialName, 255, "__rml_shader_%s_%d_[%.2f,%.2f]_[%.2f,%.2f]_%08X",
            name.c_str(), repeating ? "_repeat_" : "_norepeat_",
            p0.x, p0.y, p1.x, p1.y, hash);

        Q_snprintf(p, 32, "[%.2f %.2f]", p0.x, p0.y);
        Q_snprintf(v, 32, "[%.2f %.2f]", p1.x, p1.y);
     
        pVMTKeyValues->SetString("$P", p);
        pVMTKeyValues->SetString("$V", v);

        pVMTKeyValues->SetInt("$NUM_STOPS", colorStopList.size());

        // We loop through stop list
        // but keep in mind, shader supports only 8 stop colors
        // because of SM2
        //
        // STOP_COLOR_[1-8]
        for (size_t i = 0; i < colorStopList.size(); i++)
        {
            const Rml::ColorStop& stop = colorStopList[i];

            char stopColorParam[16];
            Q_snprintf(stopColorParam, 16, "$STOP_COLOR_%i", i + 1);

            char stopPositionParam[32];
            Q_snprintf(stopPositionParam, 32, "$STOP_POSITION_%i", i + 1);

            char stopColorValue[32];
            Q_snprintf(stopColorValue, 32, "[%.2f %.2f %.2f %.2f]",
                stop.color.red / 255.f,
                stop.color.green / 255.f,
                stop.color.blue / 255.f,
                stop.color.alpha / 255.f
            );

            pVMTKeyValues->SetString(stopColorParam, stopColorValue);
            pVMTKeyValues->SetFloat(stopPositionParam, stop.position.number);
        }

        IMaterial* mat = g_pMaterialSystem->FindProceduralMaterial(pMaterialName, TEXTURE_GROUP_OTHER, pVMTKeyValues);
        shaderHandle->material.Init(mat);
    }
    else
    {
        Warning("RmlUi: Unsupported custom shader %s name!", name.c_str());
    }

    return reinterpret_cast<Rml::CompiledShaderHandle>(shaderHandle);
}

void RmlUIRenderInterface::RenderShader(Rml::CompiledShaderHandle shader, Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation, Rml::TextureHandle /*texture*/)
{
    Rml::TextureHandle textureHandle = reinterpret_cast<Rml::TextureHandle>(reinterpret_cast<TextureHandle*>(shader));

    if (!textureHandle)
        return;

    CMatRenderContextPtr pRenderContext(materials);
    
    RenderGeometry(geometry, translation, textureHandle);
}

void RmlUIRenderInterface::ReleaseShader(Rml::CompiledShaderHandle shader)
{
    Rml::TextureHandle textureHandle = reinterpret_cast<Rml::TextureHandle>(reinterpret_cast<TextureHandle*>(shader));

    if (!textureHandle)
        return;

    ReleaseTexture(textureHandle);
}

/// Disable transform
void RmlUIRenderInterface::DisableTransform()
{
    transformEnabled = false;
}

/// Generate texture
void RmlUIProceduralRegenerator::RegenerateTextureBits(ITexture* pTexture, IVTFTexture* pVTFTexture, Rect_t* pSubRect)
{
    CPixelWriter pixelWriter;
    pixelWriter.SetPixelMemory(pVTFTexture->Format(),
        pVTFTexture->ImageData(0, 0, 0), pVTFTexture->RowSizeInBytes(0));

    if (!dataBuffer.size())
        Error("RmlUIProceduralRegenerator: Failed to regenerate texture bits - data is invalid");

    int bytesPerPixel = 4; // 4 bytes per pixel RGBA
    int stride = pVTFTexture->Width() * bytesPerPixel; // Row size in bytes
    int dataSize = stride * pVTFTexture->Height(); // Total texture size in bytes

    int xmax = pSubRect->x + pSubRect->width; // Right bound
    int ymax = pSubRect->y + pSubRect->height; // Top bound

    for (int y = pSubRect->y; y < ymax; ++y)
    {
        pixelWriter.Seek(pSubRect->x, y);

        for (int x = pSubRect->x; x < xmax; ++x)
        {
            int srcIndex = (y * stride) + (x * bytesPerPixel);

            if (srcIndex < 0 || (srcIndex + 3) >= dataSize)
            {
                Error("RmlUIProceduralRegenerator: Out of bounds read at index %d (max: %d)\n", srcIndex, dataSize);
                return;
            }

            Rml::byte r = dataBuffer[srcIndex + 0];
            Rml::byte g = dataBuffer[srcIndex + 1];
            Rml::byte b = dataBuffer[srcIndex + 2];
            Rml::byte a = dataBuffer[srcIndex + 3];

            pixelWriter.WritePixel(r, g, b, a);
        }
    }
}

/// Release (nothing to remove)
void RmlUIProceduralRegenerator::Release()
{
}

/// Set data
void RmlUIProceduralRegenerator::SetData(const Rml::byte* _data, size_t dataSize)
{
    dataBuffer.assign(_data, _data + dataSize);
    dataBuffer.shrink_to_fit();
}
