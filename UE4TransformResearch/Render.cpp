#include "main.h"

#include "DXUTgui.h"
#include "SDKmisc.h"

extern GraphicResources * G;

extern SwapChainGraphicResources * SCG;

extern SceneState scene_state;

extern BlurHandling blur_handling;

extern CDXUTTextHelper*                    g_pTxtHelper;

ID3D11ShaderResourceView* null[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

inline void set_scene_constant_buffer(ID3D11DeviceContext* context){
	G->scene_constant_buffer->SetData(context, scene_state);
};

inline void set_blur_constant_buffer(ID3D11DeviceContext* context){
	//G->blur_constant_buffer->SetData(context, blur_handling);
};

void RenderText()
{
	g_pTxtHelper->Begin();
	g_pTxtHelper->SetInsertionPos(2, 0);
	g_pTxtHelper->SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
	g_pTxtHelper->DrawTextLine(DXUTGetFrameStats(true && DXUTIsVsyncEnabled()));
	g_pTxtHelper->DrawTextLine(DXUTGetDeviceStats());

	g_pTxtHelper->End();
}

void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	Camera::OnFrameMove(fTime, fElapsedTime, pUserContext);
}

void renderSceneIntoGBuffer(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext);
void postProccessGBuffer(ID3D11Device* pd3dDevice, ID3D11DeviceContext* context);
void postProccessBlur(ID3D11Device* pd3dDevice, ID3D11DeviceContext* context, _In_opt_ std::function<void __cdecl()> setHState, _In_opt_ std::function<void __cdecl()> setVState);

void clearAndSetRenderTarget(ID3D11DeviceContext* context, float ClearColor[], int n, ID3D11RenderTargetView** pRTV, ID3D11DepthStencilView* pDSV){
	for (int i = 0; i < n; i++)
		context->ClearRenderTargetView(pRTV[i], ClearColor);

	context->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	context->OMSetRenderTargets(n, pRTV, pDSV); //renderTargetViewToArray(pRTV) DXUTGetD3D11RenderTargetView
}
void plane_set_world_matrix(SimpleMath::Matrix w);
void plane_draw(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState);
void DrawQuad(ID3D11DeviceContext* pd3dImmediateContext, _In_ IEffect* effect, _In_opt_ std::function<void __cdecl()> setCustomState);
/////
struct Transformation{
	SimpleMath::Vector3 scale;
	SimpleMath::Vector3 rotate;
	SimpleMath::Vector3 translate;
	float degToRad(float d){
		return (d * 3.141592654f) / 180.0f;
	}
	Transformation(){
		scale = SimpleMath::Vector3(1.0f, 1.0f, 1.0f);
	}
	//SimpleMath::Vector3 operator*(const SimpleMath::Vector3& scale){
	//	return	SimpleMath::Vector3(
	//		a.x*b.x*c.x*d.x*e.x,
	//		a.y*b.y*c.y*d.y*e.y,
	//		a.z*b.z*c.z*d.z*e.z
	//		);
	//}
	SimpleMath::Matrix matrix(SimpleMath::Vector3 prev_scale = SimpleMath::Vector3(1, 1, 1)){
		SimpleMath::Matrix result_matrix =
			SimpleMath::Matrix::CreateScale(scale)*
			SimpleMath::Matrix::CreateScale(prev_scale)*

			SimpleMath::Matrix::CreateRotationZ(degToRad(rotate.z))*
			SimpleMath::Matrix::CreateRotationY(degToRad(rotate.y))*
			SimpleMath::Matrix::CreateRotationX(degToRad(rotate.x))*

			SimpleMath::Matrix::CreateScale(1.0f / prev_scale.x, 1.0f / prev_scale.y, 1.0f / prev_scale.z)*

			SimpleMath::Matrix::CreateTranslation(translate);

		return result_matrix;
	}
};
SimpleMath::Vector3 operator* (SimpleMath::Vector3 & a, SimpleMath::Vector3 & b){
	return
		SimpleMath::Vector3(
			a.x*b.x,
			a.y*b.y,
			a.z*b.z
	);
}
//SimpleMath::Vector3 combineScale(
//	SimpleMath::Vector3 a = SimpleMath::Vector3(1, 1, 1),
//	SimpleMath::Vector3 b = SimpleMath::Vector3(1, 1, 1),
//	SimpleMath::Vector3 c = SimpleMath::Vector3(1, 1, 1),
//	SimpleMath::Vector3 d = SimpleMath::Vector3(1, 1, 1),
//	SimpleMath::Vector3 e = SimpleMath::Vector3(1, 1, 1)){
//	return
//		SimpleMath::Vector3(
//		a.x*b.x*c.x*d.x*e.x,
//		a.y*b.y*c.y*d.y*e.y,
//		a.z*b.z*c.z*d.z*e.z
//		);
//}
/////
Transformation Frames[5];
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* context,
	double fTime, float fElapsedTime, void* pUserContext)
{
	///
	Frames[0].translate = SimpleMath::Vector3(0, 0, 0);
	Frames[0].rotate = SimpleMath::Vector3(0, 0, 0);
	Frames[0].scale = SimpleMath::Vector3(5, 5, 5);

	Frames[1].translate = SimpleMath::Vector3(38, 0, 0);
	Frames[1].rotate = SimpleMath::Vector3(0, 0, 35.999901);
	Frames[1].scale = SimpleMath::Vector3(1.5, 2, 2);

	Frames[2].translate = SimpleMath::Vector3(26.66667, 26.666668, 0);
	Frames[2].rotate = SimpleMath::Vector3(0, 0, 0);
	Frames[2].scale = SimpleMath::Vector3(2, 1, 1);

	Frames[3].translate = SimpleMath::Vector3(-13.53828, -63.0, 0);
	Frames[3].rotate = SimpleMath::Vector3(0, 0, -35);
	Frames[3].scale = SimpleMath::Vector3(1, 1, 1);

	Frames[4].translate = SimpleMath::Vector3(10, 10, 0);
	Frames[4].rotate = SimpleMath::Vector3(0,0,0);
	Frames[4].scale = SimpleMath::Vector3(1, 1, 1);
	///

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = scene_state.vFrustumParams.x;
	vp.Height = scene_state.vFrustumParams.y;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	context->RSSetViewports(1, &vp);

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	{
		context->PSSetShaderResources(0, 5, null);
	}
	{
		clearAndSetRenderTarget(context, clearColor, 1, renderTargetViewToArray(DXUTGetD3D11RenderTargetView()), DXUTGetD3D11DepthStencilView());
	
		context->VSSetConstantBuffers(0, 1, constantBuffersToArray(*(G->scene_constant_buffer)));

		DrawQuad(context, G->quad_effect.get(), [=]{
			context->PSSetShaderResources(0, 1, shaderResourceViewToArray(G->ref_texture.Get()));
			context->PSSetSamplers(0, 1, samplerStateToArray(G->render_states->AnisotropicWrap()));

			context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
			context->RSSetState(G->render_states->CullNone());
			context->OMSetDepthStencilState(G->render_states->DepthNone(), 0);
		});

		plane_set_world_matrix(Frames[1].matrix(Frames[0].scale)*Frames[0].matrix());
		set_scene_constant_buffer(context);
		plane_draw(context, G->model_effect.get(), G->model_input_layout.Get(), [=]{
			context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
			context->RSSetState(G->render_states->Wireframe());
			context->OMSetDepthStencilState(G->render_states->DepthDefault(), 0);
		});

		plane_set_world_matrix(Frames[2].matrix(Frames[0].scale*Frames[1].scale)*Frames[1].matrix(Frames[0].scale)*Frames[0].matrix());
		set_scene_constant_buffer(context);
		plane_draw(context, G->model_effect.get(), G->model_input_layout.Get(), [=]{
			context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
			context->RSSetState(G->render_states->Wireframe());
			context->OMSetDepthStencilState(G->render_states->DepthDefault(), 0);
		});

		plane_set_world_matrix(Frames[3].matrix(Frames[0].scale*Frames[1].scale*Frames[2].scale)*Frames[2].matrix(Frames[0].scale*Frames[1].scale)*Frames[1].matrix(Frames[0].scale)*Frames[0].matrix());
		set_scene_constant_buffer(context);
		plane_draw(context, G->model_effect.get(), G->model_input_layout.Get(), [=]{
			context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
			context->RSSetState(G->render_states->Wireframe());
			context->OMSetDepthStencilState(G->render_states->DepthDefault(), 0);
		});

		plane_set_world_matrix(Frames[4].matrix(Frames[0].scale*Frames[1].scale*Frames[2].scale*Frames[3].scale)*Frames[3].matrix(Frames[0].scale*Frames[1].scale*Frames[2].scale)*Frames[2].matrix(Frames[0].scale*Frames[1].scale)*Frames[1].matrix(Frames[0].scale)*Frames[0].matrix());
		set_scene_constant_buffer(context);
		plane_draw(context, G->model_effect.get(), G->model_input_layout.Get(), [=]{
			context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
			context->RSSetState(G->render_states->Wireframe());
			context->OMSetDepthStencilState(G->render_states->DepthDefault(), 0);
		});

	}
	{
		context->PSSetShaderResources(0, 5, null);
	}

	RenderText();
}

