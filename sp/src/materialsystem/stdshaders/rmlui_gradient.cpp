// ==================================================================
// Den Urakolouy AKA URAKOLOUY5
// 2025
// 
// Feel free to use it as you want to use.
// Major code based on open source references from Source SDK.
// ==================================================================

// Must include this. Contains a bunch of macro definitions along with the
// declaration of CBaseShader.
#include "BaseVSShader.h"

// We also need to include the pixel shader for our own shader.
// Note that the shader compiler >2.0b versions.
#include "rmlui_gradient_ps20b.inc"
#include "rmlui_gradient_vs20.inc"

/// This macro defines the start of the shader.
BEGIN_SHADER( RmlUi_Gradient, "Help for RmlUi gradient shader." )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM(FUNC, SHADER_PARAM_TYPE_INTEGER, "0", "Gradient function type (0 = LINEAR, etc.)")
		SHADER_PARAM(P, SHADER_PARAM_TYPE_VEC2, "[0.0 0.0]", "Starting point / center")
		SHADER_PARAM(V, SHADER_PARAM_TYPE_VEC2, "[1.0 0.0]", "Vector to ending point / curvature / angle")
    
		SHADER_PARAM(STOP_COLOR_1, SHADER_PARAM_TYPE_COLOR, "[0.0 0.0 0.0 1.0]", "Color at stop 1")
		SHADER_PARAM(STOP_COLOR_2, SHADER_PARAM_TYPE_COLOR, "[1.0 1.0 1.0 1.0]", "Color at stop 2")
		SHADER_PARAM(STOP_COLOR_3, SHADER_PARAM_TYPE_COLOR, "[1.0 0.0 0.0 1.0]", "Color at stop 3")
		SHADER_PARAM(STOP_COLOR_4, SHADER_PARAM_TYPE_COLOR, "[0.0 1.0 0.0 1.0]", "Color at stop 4")
		SHADER_PARAM(STOP_COLOR_5, SHADER_PARAM_TYPE_COLOR, "[0.0 0.0 1.0 1.0]", "Color at stop 5")
		SHADER_PARAM(STOP_COLOR_6, SHADER_PARAM_TYPE_COLOR, "[1.0 1.0 0.0 1.0]", "Color at stop 6")
		SHADER_PARAM(STOP_COLOR_7, SHADER_PARAM_TYPE_COLOR, "[0.0 1.0 1.0 1.0]", "Color at stop 7")
		SHADER_PARAM(STOP_COLOR_8, SHADER_PARAM_TYPE_COLOR, "[1.0 0.0 1.0 1.0]", "Color at stop 8")

		SHADER_PARAM(STOP_POSITION_1, SHADER_PARAM_TYPE_FLOAT, "0.0", "Position of stop 1")
		SHADER_PARAM(STOP_POSITION_2, SHADER_PARAM_TYPE_FLOAT, "1.0", "Position of stop 2")
		SHADER_PARAM(STOP_POSITION_3, SHADER_PARAM_TYPE_FLOAT, "0.25", "Position of stop 3")
		SHADER_PARAM(STOP_POSITION_4, SHADER_PARAM_TYPE_FLOAT, "0.375", "Position of stop 4")
		SHADER_PARAM(STOP_POSITION_5, SHADER_PARAM_TYPE_FLOAT, "0.5", "Position of stop 5")
		SHADER_PARAM(STOP_POSITION_6, SHADER_PARAM_TYPE_FLOAT, "0.625", "Position of stop 6")
		SHADER_PARAM(STOP_POSITION_7, SHADER_PARAM_TYPE_FLOAT, "0.75", "Position of stop 7")
		SHADER_PARAM(STOP_POSITION_8, SHADER_PARAM_TYPE_FLOAT, "0.875", "Position of stop 8")
    
		SHADER_PARAM(NUM_STOPS, SHADER_PARAM_TYPE_INTEGER, "2", "Number of stops")
	END_SHADER_PARAMS

	/// Shader init for params
	SHADER_INIT_PARAMS()
	{
		if (!params[FUNC]->IsDefined())
			params[FUNC]->SetIntValue(0);

		if (!params[P]->IsDefined())
			params[P]->SetVecValue(0.0f, 0.0f);

		if (!params[V]->IsDefined())
			params[V]->SetVecValue(1.0f, 0.0f);

		if (!params[STOP_COLOR_1]->IsDefined())
			params[STOP_COLOR_1]->SetVecValue(0.0f, 0.0f, 0.0f, 1.0f);

		if (!params[STOP_COLOR_2]->IsDefined())
			params[STOP_COLOR_2]->SetVecValue(1.0f, 1.0f, 1.0f, 1.0f);

		if (!params[NUM_STOPS]->IsDefined())
			params[NUM_STOPS]->SetIntValue(2);
	}

	/// Shader init block
	SHADER_INIT
	{
	}

	/// We want this shader to operate on the frame buffer itself. Therefore,
	/// we need to set this to true.
	bool NeedsFullFrameBufferTexture(IMaterialVar **params, bool bCheckSpecificToThisFrame /* = true */) const
	{
		return true;
	}

	// Setup fallback to Wireframe
	SHADER_FALLBACK
	{
		// Requires DX9 + above
		if (g_pHardwareConfig->GetDXSupportLevel() < 90)
		{
			Assert(0);
			return "Wireframe";
		}
		return 0;
	}

	// Drawing our shader
	SHADER_DRAW
	{
		// Initial shader bind
		SHADOW_STATE
		{
			// Setup the vertex format.
			int fmt = VERTEX_POSITION | VERTEX_COLOR;
			pShaderShadow->VertexShaderVertexFormat(fmt, 1, 0, 0);

			// Enable alpha blend
			EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);

			// Precache and set the shaders.
			DECLARE_STATIC_VERTEX_SHADER(rmlui_gradient_vs20);
			SET_STATIC_VERTEX_SHADER(rmlui_gradient_vs20);

			DECLARE_STATIC_PIXEL_SHADER(rmlui_gradient_ps20b);
			SET_STATIC_PIXEL_SHADER(rmlui_gradient_ps20b);
		}

		// Called every frame
		DYNAMIC_STATE
		{
			// FUNC and NUM_STOPS
			int func = params[FUNC]->GetIntValue();
			int numStops = params[NUM_STOPS]->GetIntValue();
			float c0[4] = { static_cast<float>(func), static_cast<float>(numStops), 0.0f, 0.0f };
			pShaderAPI->SetPixelShaderConstant(0, c0, 1);

			// P and V
			float p[4];
			params[P]->GetVecValue(p, 2);
			float v[4];
			params[V]->GetVecValue(v, 2);
			float c1[4] = { p[0], p[1], v[0], v[1] };
			pShaderAPI->SetPixelShaderConstant(1, c1, 1);

			// STOP_COLORS (Handles 8 color stops)
			for (int i = 0; i < 8; ++i)
			{
				float color[4];

				if (i == 0) params[STOP_COLOR_1]->GetVecValue(color, 4);
				else if (i == 1) params[STOP_COLOR_2]->GetVecValue(color, 4);
				else if (i == 2) params[STOP_COLOR_3]->GetVecValue(color, 4);
				else if (i == 3) params[STOP_COLOR_4]->GetVecValue(color, 4);
				else if (i == 4) params[STOP_COLOR_5]->GetVecValue(color, 4);
				else if (i == 5) params[STOP_COLOR_6]->GetVecValue(color, 4);
				else if (i == 6) params[STOP_COLOR_7]->GetVecValue(color, 4);
				else if (i == 7) params[STOP_COLOR_8]->GetVecValue(color, 4);

				pShaderAPI->SetPixelShaderConstant(2 + i, color, 1);
			}

			// STOP_POSITIONS (Handles 8 stop positions)
			for (int i = 0; i < 8; ++i)
			{
				float stopPosition;

				if (i == 0) stopPosition = params[STOP_POSITION_1]->GetFloatValue();
				else if (i == 1) stopPosition = params[STOP_POSITION_2]->GetFloatValue();
				else if (i == 2) stopPosition = params[STOP_POSITION_3]->GetFloatValue();
				else if (i == 3) stopPosition = params[STOP_POSITION_4]->GetFloatValue();
				else if (i == 4) stopPosition = params[STOP_POSITION_5]->GetFloatValue();
				else if (i == 5) stopPosition = params[STOP_POSITION_6]->GetFloatValue();
				else if (i == 6) stopPosition = params[STOP_POSITION_7]->GetFloatValue();
				else if (i == 7) stopPosition = params[STOP_POSITION_8]->GetFloatValue();

				pShaderAPI->SetPixelShaderConstant(10 + i, &stopPosition, 1);
			}

			// Use our custom vertex shader.
			DECLARE_DYNAMIC_VERTEX_SHADER(rmlui_gradient_vs20);
			SET_DYNAMIC_VERTEX_SHADER(rmlui_gradient_vs20);

			// Use our custom pixel shader.
			DECLARE_DYNAMIC_PIXEL_SHADER(rmlui_gradient_ps20b);
			SET_DYNAMIC_PIXEL_SHADER(rmlui_gradient_ps20b);
		}

		// Draw our shader now!
		Draw();
	}

END_SHADER