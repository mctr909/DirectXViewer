#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <fstream>
#include <Shlwapi.h>

// D3D
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// Comptr
#include <atlcomcli.h>

#include "graphic.h"

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

/********************** [ コンストラクタ/デストラクタ ] ***********************/
DXAPI::Graphic::
Graphic() {
}

DXAPI::Graphic::
~Graphic() {
}

/***************************** [ publicメソッド ] *****************************/
void DXAPI::Graphic::
SetWindowHandle(HWND hWnd) {
	mWindowHandle = hWnd;
}

bool DXAPI::Graphic::
Initialize(int32_t w, int32_t h) {
	if(w == 0 || h == 0) {
		return false;
	}

	if(!CreateDeviceAndSwapChain(w, h)) {
		ReleaseComPtr();
		return false;
	}

	if(!CreateRenderTarget()) {
		ReleaseComPtr();
		return false;
	}

	if(!CreateDefaultRasterizerState()) {
		ReleaseComPtr();
		return false;
	}

	if(!CreateDepthStencilState()) {
		ReleaseComPtr();
		return false;
	}

	if(!CreateStencilBuffer(w, h)) {
		ReleaseComPtr();
		return false;
	}

	// レンダーターゲットに深度/ステンシルテクスチャを設定
	mContext->OMSetRenderTargets(1, &mRtv.p, mDsv);
	// ビューポートの設定
	D3D11_VIEWPORT vp[] = {
		{ 0, 0, static_cast<FLOAT>(w), static_cast<FLOAT>(h), 0, 1.0f }
	};
	mContext->RSSetViewports(1, vp);

	if(!CreateShaderFromCompiledFiles()) {
		ReleaseComPtr();
		return false;
	}

	if(!CreateConstantBuffer()) {
		ReleaseComPtr();
		return false;
	}

	mWidth = w;
	mHeight = h;

	return true;
}

bool DXAPI::Graphic::
Resize(int32_t w, int32_t h) {
	if(w == 0 || h == 0 || mDevice == nullptr) {
		return false;
	}

	ID3D11RenderTargetView *irtv = nullptr;
	mContext->OMSetRenderTargets(1, &irtv, nullptr);
	mRtv.Release();
	mBackbuffer.Release();

	mDsv.Release();
	mDepthtex.Release();

	if(FAILED(mSwapchain->ResizeBuffers(mSwapchaincount, w, h, SWAPCHAIN_FORMAT, 0))) {
		return false;
	}

	if(!CreateRenderTarget()) {
		ReleaseComPtr();
		return false;
	}

	if(!CreateStencilBuffer(w, h)) {
		ReleaseComPtr();
		return false;
	}

	// レンダーターゲットに深度/ステンシルテクスチャを設定
	mContext->OMSetRenderTargets(1, &mRtv.p, mDsv);
	// ビューポートの設定
	D3D11_VIEWPORT vp[] = {
		{ 0, 0, static_cast<FLOAT>(w), static_cast<FLOAT>(h), 0, 1.0f }
	};
	mContext->RSSetViewports(1, vp);

	mMatProj = DirectX::XMMatrixPerspectiveFovRH(static_cast<float>((M_PI / 4)), 1.0f * w / h, NEAR_Z, FAR_Z);

	mWidth = w;
	mHeight = h;

	return true;
}

void DXAPI::Graphic::
SetCamera(float distance, float azimuth, float elevation) {
	mMatProj = DirectX::XMMatrixPerspectiveFovRH(azimuth, 1.0f * mWidth / mHeight, NEAR_Z, FAR_Z);

	mCameraPosition = Vec3(
		distance * sinf(elevation) * cosf(azimuth),
		-distance * sinf(elevation) * sinf(azimuth),
		distance * cosf(elevation)
	);

	mLokatpoint = Vec3(0.0f, 0.0f, -30.0f);

	float upsetz = mCamupset ? -1.0f : 1.0f;
	DirectX::XMVECTOR eye = DirectX::XMVectorSet(mCameraPosition.x, mCameraPosition.y, mCameraPosition.z, 0.0f);
	DirectX::XMVECTOR focus = DirectX::XMVectorSet(mLokatpoint.x, mLokatpoint.y, mLokatpoint.z, 0.0f);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 0.0f, upsetz, 0.0f);
	mMatView = DirectX::XMMatrixLookAtRH(eye, focus, up);
}

void DXAPI::Graphic::
Render() {
	UINT strides = sizeof(CoordColor);
	UINT offset = 0;

	if(mContext == nullptr) {
		return;
	}

	// バックバッファと深度バッファのクリア
	FLOAT backcolor[4] = {1.0f, 0.90f, 0.8f, 1.f};
	mContext->ClearRenderTargetView(mRtv, backcolor);
	mContext->ClearDepthStencilView(mDsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// 頂点データに渡すデータのレイアウトを設定
	mContext->IASetInputLayout(mInputLayout);

	// 頂点シェーダー, ジオメトリシェーダー, ピクセルシェーダーの設定
	mContext->VSSetShader(mVertexShader, nullptr, 0);
	mContext->GSSetShader(mGeometryShader, nullptr, 0);
	mContext->PSSetShader(mPixelShader, nullptr, 0);

	// ラスタライザーステートを設定
	mContext->RSSetState(mRs);

	MatrixBuffer matrixbuf = {
		// シェーダーでは列優先(column_major)で行列データを保持するため, 転置を行う
		DirectX::XMMatrixTranspose(mMatProj),
		DirectX::XMMatrixTranspose(mMatView),
		DirectX::XMMatrixTranspose(mMatWorld)
	};

	// マトリックスバッファの設定
	mContext->UpdateSubresource(mMatrixBuffer, 0, nullptr, &matrixbuf, 0, 0);
	mContext->VSSetConstantBuffers(0, 1, &mMatrixBuffer.p);
	mContext->GSSetConstantBuffers(0, 1, &mMatrixBuffer.p);

	// 深度・ステンシルバッファの使用方法を設定
	mContext->OMSetDepthStencilState(mDss, 0);

	// モデルを表示
	mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for(auto itrModel = mModelList.rbegin(), e = mModelList.rend(); itrModel != e; ++itrModel) {
		if(nullptr == itrModel->vertex || 0 == itrModel->visible) {
			continue;
		}
		mContext->IASetVertexBuffers(0, 1, &itrModel->vertex.p, &strides, &offset);
		mContext->IASetIndexBuffer(itrModel->index, DXGI_FORMAT_R32_UINT, 0);
		mContext->DrawIndexed(itrModel->numindices, 0, 0);
	}

	// 作成したプリミティブをウィンドウへ描画
	if(mSwapchain != nullptr) {
		mSwapchain->Present(0, 0);
	}
}

void * DXAPI::Graphic::
AddGeometryBuffers(float *lp_vert, int32_t vert_count, int32_t *lp_surf, int32_t surf_count) {
	Model model;
	model.visible = 1;

	D3D11_BUFFER_DESC bdvertex = {
		static_cast<UINT>(sizeof(float) * 7 * vert_count),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER
	};
	D3D11_SUBRESOURCE_DATA srdv = {lp_vert};
	mDevice->CreateBuffer(&bdvertex, &srdv, &model.vertex.p);

	D3D11_BUFFER_DESC bdindex = {
		static_cast<UINT>(sizeof(int32_t) * 3 * surf_count),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER
	};
	D3D11_SUBRESOURCE_DATA srdind = {lp_surf};
	mDevice->CreateBuffer(&bdindex, &srdind, &model.index.p);
	model.numindices = static_cast<UINT>(3 * surf_count);

	mModelList.push_back(model);
	return &mModelList.back();
}

void DXAPI::Graphic::
RemoveGeometryBuffers(void *lp_model) {
	Model *p_model = static_cast<Model *>(lp_model);
	p_model->vertex.Release();
	p_model->index.Release();
	mDeleteCount++;
	if(mDeleteCount >= 100) {
		std::vector<Model> temp;
		for(auto itr = mModelList.rbegin(), e = mModelList.rend(); itr != e; ++itr) {
			if(nullptr != itr->vertex) {
				temp.push_back(*itr);
			}
		}
		mModelList.clear();
		for(auto itr = temp.rbegin(), e = temp.rend(); itr != e; ++itr) {
			mModelList.push_back(*itr);
		}
		mDeleteCount = 0;
	}
}

void DXAPI::Graphic::
SetVisibleModel(void *lp_model, BYTE visible) {
	Model *p_model = static_cast<Model *>(lp_model);
	p_model->visible = visible;
}

/**************************** [ privateメソッド ] *****************************/
bool DXAPI::Graphic::
CreateDeviceAndSwapChain(int32_t w, int32_t h) {
	DXGI_SWAP_CHAIN_DESC desc = {
		{ static_cast<UINT>(w), static_cast<UINT>(h),{ 60, 1 },
		SWAPCHAIN_FORMAT, DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, DXGI_MODE_SCALING_UNSPECIFIED },
		mSampledesc,
		DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT,
		mSwapchaincount,
		mWindowHandle,
		TRUE,
		DXGI_SWAP_EFFECT_DISCARD,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};

	if (FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&desc,
		&mSwapchain.p,
		&mDevice.p,
		&FEATURE_LEVEL,
		&mContext)))
		return false;

	return true;
}

bool DXAPI::Graphic::
CreateRenderTarget() {
	if (FAILED(mSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&mBackbuffer))) {
		return false;
	}
	if (FAILED(mDevice->CreateRenderTargetView(mBackbuffer, nullptr, &mRtv))) {
		return false;
	}
	return true;
}

bool DXAPI::Graphic::
CreateDefaultRasterizerState() {
	D3D11_RASTERIZER_DESC desc = {
		D3D11_FILL_SOLID, D3D11_CULL_NONE, TRUE, 0,	0.0f, 0.0f,
		TRUE, FALSE, FALSE, FALSE
	};
	if (FAILED(mDevice->CreateRasterizerState(&desc, &mRs))) {
		return false;
	}

	return true;
}

bool DXAPI::Graphic::
CreateDepthStencilState() {
	D3D11_DEPTH_STENCIL_DESC desc = {
		TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS,
		FALSE, D3D11_DEFAULT_STENCIL_READ_MASK, D3D11_DEFAULT_STENCIL_WRITE_MASK,
		D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS
	};

	if (FAILED(mDevice->CreateDepthStencilState(&desc, &mDss))) {
		return false;
	}

	return true;
}

bool DXAPI::Graphic::
CreateStencilBuffer(int32_t w, int32_t h) {
	D3D11_TEXTURE2D_DESC texdesc = {
		static_cast<UINT>(w), static_cast<UINT>(h), 1, 1,
		DXGI_FORMAT_R24G8_TYPELESS, mSampledesc, D3D11_USAGE_DEFAULT, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE
	};

	if (FAILED(mDevice->CreateTexture2D(&texdesc, nullptr, &mDepthtex))) {
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvdesc = {
		DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_DSV_DIMENSION_TEXTURE2D
	};

	if (FAILED(mDevice->CreateDepthStencilView(mDepthtex, &dsvdesc, &mDsv))) {
		return false;
	}

	return true;
}

bool DXAPI::Graphic::
CreateShaderFromCompiledFiles() {
	auto WideStr2MultiByte = [](const std::wstring wstr) -> std::string {
		size_t size = ::WideCharToMultiByte(CP_OEMCP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		std::vector<char> buf;
		buf.resize(size);
		::WideCharToMultiByte(CP_OEMCP, 0, wstr.c_str(), -1, &buf.front(), static_cast<int>(size), nullptr, nullptr);
		std::string ret(&buf.front(), buf.size() - 1);
		return ret;
	};

	std::wstring filepath;
	filepath.resize(MAX_PATH);
	::GetModuleFileName(NULL, &filepath.front(), MAX_PATH);
	::PathRemoveFileSpec(&filepath.front());

	// vertex shader
	std::string csofile = WideStr2MultiByte(filepath);
	csofile.append("\\VertexShader.cso");
	std::ifstream ifs(csofile, std::ios::in | std::ios::binary);
	if (ifs.fail()) return false;
	ifs.seekg(0, std::ifstream::end);
	size_t csosize = static_cast<size_t>(ifs.tellg());
	ifs.seekg(0, std::ifstream::beg);
	std::vector<char> csodata;
	csodata.resize(csosize);
	ifs.read(&csodata.front(), csosize);

	if (FAILED(mDevice->CreateVertexShader(&csodata.front(), csosize, nullptr, &mVertexShader.p))) {
		return false;
	}

	// 入力するデータのレイアウトを定義
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT , 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT num = ARRAYSIZE(layout);

	if (FAILED(mDevice->CreateInputLayout(layout, num, &csodata.front(), csosize, &mInputLayout.p))) {
		return false;
	}

	// geometry shader
	ifs.close();
	csodata.clear();
	csofile = WideStr2MultiByte(filepath);
	csofile.append("\\GeometryShader.cso");
	ifs.open(csofile, std::ios::in | std::ios::binary);
	if (ifs.fail()) return false;
	ifs.seekg(0, std::ifstream::end);
	csosize = static_cast<size_t>(ifs.tellg());
	ifs.seekg(0, std::ifstream::beg);
	csodata.resize(csosize);
	ifs.read(&csodata.front(), csosize);

	if (FAILED(mDevice->CreateGeometryShader(&csodata.front(), csosize, nullptr, &mGeometryShader.p))) {
		return false;
	}

	// pixel shader
	ifs.close();
	csofile = WideStr2MultiByte(filepath);
	csofile.append("\\PixelShader.cso");
	ifs.open(csofile, std::ios::in | std::ios::binary);
	if (ifs.fail())
		return false;
	ifs.seekg(0, std::ifstream::end);
	csosize = static_cast<size_t>(ifs.tellg());
	ifs.seekg(0, std::ifstream::beg);
	csodata.clear();
	csodata.resize(csosize);
	ifs.read(&csodata.front(), csosize);

	if (FAILED(mDevice->CreatePixelShader(&csodata.front(), csosize, nullptr, &mPixelShader.p))) {
		return false;
	}

	return true;
}

bool DXAPI::Graphic::
CreateConstantBuffer() {
	// TODO : Create CB
	D3D11_BUFFER_DESC matrixdesc = {
		sizeof(MatrixBuffer),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_CONSTANT_BUFFER
	};

	if (FAILED(mDevice->CreateBuffer(&matrixdesc, nullptr, &mMatrixBuffer))) {
		return false;
	}

	return true;
}

void DXAPI::Graphic::
ReleaseComPtr() {
	mPixelShader.Release();
	mGeometryShader.Release();
	mVertexShader.Release();
	mInputLayout.Release();

	mRs.Release();
	mDss.Release();
	mDsv.Release();
	mDepthtex.Release();

	mRtv.Release();
	mBackbuffer.Release();

	mSwapchain.Release();
	mContext.Release();
	mDevice.Release();
}
