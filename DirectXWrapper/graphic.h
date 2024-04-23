#ifndef __GRAPHIC_H__
#define __GRAPHIC_H__

#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include <atlcomcli.h>

namespace DXAPI {
	class Graphic {
	private:
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
		struct Model {
			CComPtr<ID3D11Buffer> vertex;
			CComPtr<ID3D11Buffer> index;
			UINT visible = 0;
			UINT numindices = 0;
		};
		struct Vec3 {
			float x, y, z;
			Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
			Vec3(const Vec3 &v) {
				x = v.x;
				y = v.y;
				z = v.z;
			}
			Vec3(Vec3 &&v) noexcept {
				x = v.x;
				y = v.y;
				z = v.z;
			}
			~Vec3() {}
			Vec3 &operator = (const Vec3 &v) {
				x = v.x;
				y = v.y;
				z = v.z;
				return *this;
			}
		};

	private:
		const float NEAR_Z = 160.0f / 1000.0f;
		const float FAR_Z = 1600.0f;

		HWND mWindowHandle = nullptr;

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
		std::vector<Model> mModelList;
		uint32_t mDeleteCount = 0;

		// DirectX算術用マトリックス
		DirectX::XMMATRIX mMatWorld = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX mMatView = DirectX::XMMatrixIdentity();
		DirectX::XMMATRIX mMatProj = DirectX::XMMatrixIdentity();

		// カメラ位置, 注視点
		Vec3 mCameraPosition = Vec3();
		Vec3 mLokatpoint = Vec3();

		// カメラの上方向を反転させるフラグ
		bool mCamupset = false;

		// 画面サイズ
		int32_t mWidth = 320;
		int32_t mHeight = 240;

	public:
		Graphic();
		~Graphic();

	public:
		void SetWindowHandle(HWND hWnd);
		bool Initialize(int32_t w, int32_t h);
		bool Resize(int32_t w, int32_t h);
		void SetCamera(float distance, float azimuth, float elevation);
		void Render();
		void * AddGeometryBuffers(float *lp_vert, int32_t vert_count, int32_t *lp_surf, int32_t surf_count);
		void RemoveGeometryBuffers(void *lp_model);
		void SetVisibleModel(void *lp_model, BYTE visible);

	private:
		bool CreateDeviceAndSwapChain(int32_t w, int32_t h);
		bool CreateRenderTarget();
		bool CreateDefaultRasterizerState();
		bool CreateDepthStencilState();
		bool CreateStencilBuffer(int32_t w, int32_t h);
		bool CreateShaderFromCompiledFiles();
		bool CreateConstantBuffer();
		void ReleaseComPtr();
	};
}

#endif /* __GRAPHIC_H__ */
