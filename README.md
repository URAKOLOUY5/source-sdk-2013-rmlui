![rmlonsourcesdk](https://github.com/user-attachments/assets/9d5fa7ad-71f3-4fba-bb1c-99d9187ee465)

<h1 align="center">
    RmlUI Integration for Source SDK 2013
</h1>

![mockup-gmod](https://github.com/user-attachments/assets/2e69860c-36b6-4e87-8cf6-4c1fdf9573d3)
<h5 align="center">
    Mockup Garry's Mod menu featuring linear-gradient, position: absolute, display: flex
</h5>

**Note:** For implementation find `#ifdef RMLUI` in code,`client_rmlui_base.vpc` file and `src/game/client/rmlui` folder

# Rendering features:
|     | Feature         | Description                                                                               | Required by properties                                                                                                                                                                              |
| --- | --------------- | ----------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| ✅   | Basic rendering | Rendering box geometry, images, text, and basic decorators.                               | Always required                                                                                                                                                                                     |
| ✅   | Clip Masks      | Proper clipping of transformed elements and elements with rounded borders.                | `transform`{:.prop} and `perspective`{:.prop}, `border-radius`{:.prop} combined with `overflow: none`{:.value}                                                                                      |
| ✅   | Transforms      | Apply arbitrary matrix transformations to rotate, scale, skew, or translate elements.     | `transform`{:.prop} and `perspective`{:.prop}                                                                                                                                                       |
|     | Layers          | Rendering to layers, and compositing, so that render effects can be applied in isolation. | `filter`{:.prop}, `backdrop-filter`{:.prop}, `mask-image`{:.prop}, `box-shadow`{:.prop}                                                                                                             |
|     | Render Targets  | Allow a layer to be stored as a texture for later rendering.                              | `box-shadow`{:.prop}                                                                                                                                                                                |
|     | Mask Images     | Allow a layer to be stored and later used as a mask.                                      | `mask-image`{:.prop}                                                                                                                                                                                |
|     | Filters         | Applying filters during compositing.                                                      | `filter`{:.prop}, `backdrop-filter`{:.prop}, `box-shadow`{:.prop} with blur applied                                                                                                                 |
| ✅   | Shaders         | Rendering geometry with special shaders.                                                  | The following `decorator`{:.prop} types: `shader`{:.value}, `linear-gradient`{:.value}, `radial-gradient`{:.value}, `conic-gradient`{:.value}, in addition to their `repeating-`{:.value} variants. |

# System features:
|     | Feature      | Description                                                  |
| --- | ------------ | ------------------------------------------------------------ |
| ✅  | Localization | Expose localization from engine                              |
| ✅   | FileSystem   | Expose filesystem from engine (`MOD` search path)           |
| ✅   | Logging      | Expose console logging (and system messages **WIN32 ONLY**) |
| ✅   | Cursors      | Allow using `cursor` property in CSS                         |
|     | Clipboard    | Expose clipboard functionality                               |
|     | Lua    | Add lua plugin integration                               |
|     | SVG    | Add svg plugin integration                               |
| ✅   | Png    | Add libpng/other integration                               |
| ✅   | Jpeg   | Add libjpeg/other integration                               |
|     | video_services   | Add webm/bink/other integration                               |

# How to compile
1. Clone or download repository as archive
2. [**Visual Studio 2022** required] Run `creategameprojects` or `createallprojects`
3. Open created .sln file
4. Compile solution in Debug/Release configuration
5. Run mod using Steam/Source SDK 2013 Singleplayer (beta set to `upcoming`)
6. Use

# Current issues
Check **Issues** tab for more info!

# Credits
- **[source-sdk-2013-ce](https://github.com/Nbc66/source-sdk-2013-ce)** - repository is based on this project and forked off
- **[RmlUI](https://github.com/mikke89/RmlUi)** - framework itself, compiled as dynamic library using CMake (temp solution, later I want to integrate RmlUI as submodule and setup CMake here) + samples + wiki
- **[SCell555's ShaderCompiler](https://github.com/SCell555/ShaderCompile)** - standalone shadercompile without dependencies on external tools (Perl, DxSDK)
- **[stb_image.h, stb_image_resize2.h](https://github.com/nothings/stb)** - used for image decoding and resizing

# Contributors
- [Alivebyte](https://github.com/Alivebyte)

Check `thirdpartylegalnotices.txt` for licenses.