#ifndef __DLLMAIN_H__
#define __DLLMAIN_H__

struct Mat4 {
	float a[16];
};

struct Camera {
	float x, y, z;
	float azimuth;
	float elevetion;
};

struct Material {
	float r, g, b, a;
	float ka;
	float ks;
};

#ifdef __cplusplus
extern "C" {
#endif
	__declspec(dllexport) void WINAPI open(int32_t width, int32_t height, Camera *lp_camera);
	__declspec(dllexport) void WINAPI close();
	__declspec(dllexport) void *WINAPI create_model(
		Mat4 *lp_mat4,
		Material material,
		float *lp_vert, int32_t vert_count,
		int32_t *lp_surf, int32_t surf_count
	);
	__declspec(dllexport) void *WINAPI create_model_color(
		Mat4 *lp_mat4,
		float *lp_vert, int32_t vert_count,
		int32_t *lp_surf, int32_t surf_count
	);
	__declspec(dllexport) void WINAPI delete_model(void *lp_model);
	__declspec(dllexport) void WINAPI visible_model(void *lp_model, BYTE visible);
	__declspec(dllexport) void WINAPI render();
#ifdef __cplusplus
}
#endif

#endif /* __DLLMAIN_H__ */
