// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include <windows.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "graphic.h"
#include "dllmain.h"

/***************************** [ グローバル変数 ] *****************************/
HWND gh_wnd = nullptr;       // ウィンドウハンドラ
DXAPI::Graphic g_dx;         // DirectX 描画処理
Camera *gp_camera = nullptr; // カメラ
uint32_t g_model_id = 0;     // モデルID

/**************************** [ プロトタイプ宣言 ] ****************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/********************************** [ 実装 ] **********************************/
void WINAPI
open(int32_t width, int32_t height, Camera *lp_camera) {
	if(gh_wnd) {
		return;
	}
	/*** ウィンドウクラスの登録 ***/
	LPCWSTR windowClassName = L"Viewer";
	WNDCLASSW wc;
	wc.style = CS_DBLCLKS | CS_PARENTDC;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = nullptr;
	wc.hIcon = nullptr;
	wc.hCursor = LoadCursor(nullptr, (LPTSTR)IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = windowClassName;
	RegisterClassW(&wc);
	gh_wnd = CreateWindowW(windowClassName, windowClassName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, nullptr, nullptr);
	if(!gh_wnd) {
		return;
	}
	gp_camera = lp_camera;
	/*** ウィンドウを開く ***/
	ShowWindow(gh_wnd, SW_SHOWDEFAULT);
	UpdateWindow(gh_wnd);
	g_dx.SetWindowHandle(gh_wnd);
	RECT rc;
	GetClientRect(gh_wnd, &rc);
	int32_t w = rc.right - rc.left;
	int32_t h = rc.bottom - rc.top;
	if(g_dx.Initialize(w, h)) {
		g_dx.SetCamera(210.0f, static_cast<float>(M_PI / 4.0f), static_cast<float>(M_PI / 3.0f));
	}
}

void WINAPI
close() {
	if(gh_wnd) {
		DestroyWindow(gh_wnd);
		gh_wnd = nullptr;
		g_model_id = 0;
	}
}

void *WINAPI
create_model(
	Mat4 *lp_mat4,
	Material material,
	float *lp_vert, int32_t vert_count,
	int32_t *lp_surf, int32_t surf_count
) {
	return nullptr;
}

void *WINAPI
create_model_color(
	Mat4 *lp_mat4,
	float *lp_vert, int32_t vert_count,
	int32_t *lp_surf, int32_t surf_count
) {
	return g_dx.AddGeometryBuffers(lp_vert, vert_count, lp_surf, surf_count);
}

void WINAPI
delete_model(void *lp_model) {
	g_dx.RemoveGeometryBuffers(lp_model);
}

void WINAPI
visible_model(void *lp_model, BYTE visible) {
	g_dx.SetVisibleModel(lp_model, visible);
}

void WINAPI
render() {
	g_dx.Render();
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	RECT rc;
	GetClientRect(hWnd, &rc);
	int32_t w = rc.right - rc.left;
	int32_t h = rc.bottom - rc.top;
	switch(message) {
	case WM_SIZE:
		if(g_dx.Resize(w, h)) {
			g_dx.Render();
		}
		break;
	case WM_COMMAND:
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: HDC を使用する描画コードをここに追加してください...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}