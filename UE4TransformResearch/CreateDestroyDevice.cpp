#include "main.h"

#include "DXUTgui.h"
#include "SDKmisc.h"

HWND DXUTgetWindow();

GraphicResources * G;

SceneState scene_state;

BlurHandling blur_handling;

BlurParams blurParams;

std::unique_ptr<Keyboard> _keyboard;
std::unique_ptr<Mouse> _mouse;

CDXUTDialogResourceManager          g_DialogResourceManager;
CDXUTTextHelper*                    g_pTxtHelper = NULL;

#include <codecvt>
std::unique_ptr<SceneNode> loadSponza(ID3D11Device* device, ID3D11InputLayout** l, DirectX::IEffect *e);

inline float lerp(float x1, float x2, float t){
	return x1*(1.0 - t) + x2*t;
}

inline float nextFloat(float x1, float x2){
	return lerp(x1, x2, (float)std::rand() / (float)RAND_MAX);
}

void FillGrid_Indexed(std::vector<VertexPositionNormalTexture> & _vertices, std::vector<WORD> & _indices, DWORD dwWidth, DWORD dwLength,
	_In_opt_ std::function<float __cdecl(SimpleMath::Vector3)> setHeight)
{
	// Fill vertex buffer
	for (DWORD i = 0; i <= dwLength; ++i)
	{
		for (DWORD j = 0; j <= dwWidth; ++j)
		{
			VertexPositionNormalTexture    pVertex;
			pVertex.position.x = ((float)j / dwWidth);
			pVertex.position.z = setHeight(pVertex.position);
			pVertex.position.y = ((float)i / dwLength);
			_vertices.push_back(pVertex);
		}
	}
	VertexPositionNormalTexture    pVertex;

	// Fill index buffer
	int index = 0;
	for (DWORD i = 0; i < dwLength; ++i)
	{
		for (DWORD j = 0; j < dwWidth; ++j)
		{
			_indices.push_back((DWORD)(i     * (dwWidth + 1) + j));
			_indices.push_back((DWORD)((i + 1) * (dwWidth + 1) + j));
			_indices.push_back((DWORD)(i     * (dwWidth + 1) + j + 1));

			_indices.push_back((DWORD)(i     * (dwWidth + 1) + j + 1));
			_indices.push_back((DWORD)((i + 1) * (dwWidth + 1) + j));
			_indices.push_back((DWORD)((i + 1) * (dwWidth + 1) + j + 1));
		}
	}
}
void loadJet(ID3D11Device* device, std::vector<VertexPositionNormalTexture> & _vertices, std::vector<uint16_t> & _indices);
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* device, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	std::srand(unsigned(std::time(0)));

	HRESULT hr;

	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();

	G = new GraphicResources();
	G->render_states = std::make_unique<CommonStates>(device);
	G->scene_constant_buffer = std::make_unique<ConstantBuffer<SceneState> >(device);

	_keyboard = std::make_unique<Keyboard>();
	_mouse = std::make_unique<Mouse>();
	HWND hwnd = DXUTgetWindow();
	_mouse->SetWindow(hwnd);

	g_DialogResourceManager.OnD3D11CreateDevice(device, context);
	g_pTxtHelper = new CDXUTTextHelper(device, context, &g_DialogResourceManager, 15);

	//effects
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"ModelVS.hlsl", L"QUAD_VS", L"vs_5_0" };
		shaderDef[L"GS"] = { L"ModelVS.hlsl", L"QUAD_GS", L"gs_5_0" };
		shaderDef[L"PS"] = { L"ModelPS.hlsl", L"QUAD_FRAG", L"ps_5_0" };

		G->quad_effect = createHlslEffect(device, shaderDef);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"ModelVS.hlsl", L"MODEL_VERTEX", L"vs_5_0" };
		shaderDef[L"PS"] = { L"ModelPS.hlsl", L"MODEL_FRAG", L"ps_5_0" };

		G->model_effect = createHlslEffect(device, shaderDef);
	}
	/*
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"blur.hlsl", L"VS", L"vs_5_0" };
		shaderDef[L"GS"] = { L"blur.hlsl", L"GS", L"gs_5_0" };
		shaderDef[L"PS"] = { L"blur.hlsl", L"HB", L"ps_5_0" };

		G->blur_h_effect = createHlslEffect(device, shaderDef);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"blur.hlsl", L"VS", L"vs_5_0" };
		shaderDef[L"GS"] = { L"blur.hlsl", L"GS", L"gs_5_0" };
		shaderDef[L"PS"] = { L"blur.hlsl", L"VB", L"ps_5_0" };

		G->blur_v_effect = createHlslEffect(device, shaderDef);
	}
	*/
	//models
	{
		G->plane_model = CreateModelMeshPart(device, [=](std::vector<VertexPositionNormalTexture> & _vertices, std::vector<uint16_t> & _indices){
			loadJet(device, _vertices, _indices);
			//FillGrid_Indexed(_vertices, _indices, 1, 1, [=](SimpleMath::Vector3 p){
			//	return 0;
			//});
		});

		WCHAR wcPath[256];
		DXUTFindDXSDKMediaFileCch(wcPath, 256, L"Textures\\Referense.png");
		hr = D3DX11CreateShaderResourceViewFromFile(device, wcPath, NULL, NULL, G->ref_texture.ReleaseAndGetAddressOf(), NULL);

		G->plane_model->CreateInputLayout(device, G->model_effect.get(), G->model_input_layout.ReleaseAndGetAddressOf());
	}

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	delete g_pTxtHelper;

	g_DialogResourceManager.OnD3D11DestroyDevice();

	_mouse = 0;

	_keyboard = 0;

	delete G;
}
