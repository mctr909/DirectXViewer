#pragma once
// M_PI
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <fstream>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

// D3D
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

// Comptr
#include <atlcomcli.h>

// Core Math Part
#include "Vector.h"

// Geometry
#include "GeometryGenerator.h"

namespace DXGraphicAPI {
	class CDxGraphic {
	private:
		HWND m_WindowHandle = NULL;
		// 機能レベル, フォーマット
		D3D_FEATURE_LEVEL FEATURE_LEVEL = D3D_FEATURE_LEVEL_11_0;
		DXGI_FORMAT SWAPCHAIN_FORMAT = DXGI_FORMAT_B8G8R8A8_UNORM;
		DXGI_FORMAT DEPTHSTENCIL_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;
		UINT mSwapchaincount = 1;
		DXGI_SAMPLE_DESC mSampledesc = { 1, 0 };

		// コアとなる処理を行うための変数
		CComPtr<ID3D11Device> mDevice;
		CComPtr<ID3D11DeviceContext> mContext;
		CComPtr<IDXGISwapChain> mSwapchain;
		CComPtr<ID3D11Texture2D> mBackbuffer;
		CComPtr<ID3D11RenderTargetView> mRtv;
		CComPtr<ID3D11Texture2D> mDepthtex;
		CComPtr<ID3D11DepthStencilView> mDsv;
		CComPtr<ID3D11RasterizerState> mRs;
		CComPtr<ID3D11DepthStencilState> mDss;
		CComPtr<ID3D11VertexShader> mVertexShader;
		CComPtr<ID3D11GeometryShader> mGeometryShader;
		CComPtr<ID3D11PixelShader> mPixelShader;
		CComPtr<ID3D11InputLayout> mInputLayout;

		// バッファ
		CComPtr<ID3D11Buffer> mMatrixBuffer;
		CComPtr<ID3D11Buffer> mVertexBuffer;
		CComPtr<ID3D11Buffer> mIndexBuffer;

		UINT mNumindices = 0;

		// DirectX算術用マトリックス
		DirectX::XMMATRIX mMatWorld = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX mMatView = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX mMatProj = DirectX::XMMatrixIdentity();

		// カメラ位置, 注視点
		Math::Vector3 mCameraPosition = Math::Vector3();
		Math::Vector3 mLokatpoint = Math::Vector3();

		// カメラの上方向を反転させるフラグ
		bool mCamupset = false;

		struct Vertex {
			float position[3];  // (x, y, z)
			float color[4];     // (r, g, b, a)
		};

		struct CoordColor {
			DirectX::XMFLOAT3 coord;
			DirectX::XMFLOAT4 color;
		};

		struct MatrixBuffer {
			DirectX::XMMATRIX matproj;
			DirectX::XMMATRIX matview;
			DirectX::XMMATRIX matworld;
		};

		bool CreateDeviceAndSwapChain(int w, int h);

		bool CreateRenderTarget();

		bool CreateDefaultRasterizerState();

		bool CreateDepthStencilState();

		bool CreateStencilBuffer(int w, int h);

		bool CreateShaderFromCompiledFiles();

		bool CreateConstantBuffer();

		void ReleaseComPtr();

	public:
		CDxGraphic();
		~CDxGraphic();

		void SetWindowHandle(HWND hWnd);

		bool InitD3D(int w, int h);

		void Render();

		void UpdateMatrices(int w, int h);

		bool ResizeView(int w, int h);

		void BuildGeometryBuffers(int windowwidth, int windowheight);
	};
}
