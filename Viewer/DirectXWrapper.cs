using System;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime.InteropServices;

namespace DirectX {
	struct Camera {
		public float x;
		public float y;
		public float z;
		public float azimuth;
		public float elevetion;
	};
	struct Material {
		public float r;
		public float g;
		public float b;
		public float a;
		public float ka;
		public float ks;
	};
	struct Surface {
		public int a;
		public int b;
		public int c;
	};

	class Model {
		public IntPtr Id = IntPtr.Zero;
		public float[] Matrix = new float[16] {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		};
	}

	class Api {
		[DllImport("DirectXWrapper.dll")]
		private static extern void open(int width, int height, IntPtr lp_camera);
		[DllImport("DirectXWrapper.dll")]
		public static extern void close();
		[DllImport("DirectXWrapper.dll")]
		private static extern IntPtr create_model(
			IntPtr lp_mat4,
			Material material,
			IntPtr lp_vert, int vert_count,
			IntPtr lp_surf, int surf_count
		);
		[DllImport("DirectXWrapper.dll")]
		private static extern IntPtr create_model_color(
			IntPtr lp_mat4,
			IntPtr lp_vert, int vert_count,
			IntPtr lp_surf, int surf_count
		);
		[DllImport("DirectXWrapper.dll")]
		public static extern void delete_model(IntPtr lp_model);
		[DllImport("DirectXWrapper.dll")]
		public static extern void visible_model(IntPtr lp_model, bool visible);
		[DllImport("DirectXWrapper.dll")]
		public static extern void render();

		public unsafe static void open(int width, int height, ref Camera camera) {
			IntPtr pCamera;
			fixed (Camera* p = &camera) {
				pCamera = (IntPtr)p;
			}
			open(width, height, pCamera);
		}

		public static void create_model(Model model, Material material, List<Vec3> vertex, List<Surface> surface) {
			create_model(model, material, vertex.ToArray(), surface.ToArray());
		}

		public static void create_model(Model model, List<Color> vertexColor, List<Vec3> vertex, List<Surface> surface) {
			create_model(model, vertexColor.ToArray(), vertex.ToArray(), surface.ToArray());
		}

		public unsafe static void create_model(Model model, Material material, Vec3[] vertex, Surface[] surface) {
			IntPtr pMat4;
			fixed (float* p = &model.Matrix[0]) {
				pMat4 = (IntPtr)p;
			}

			var arrVert = new float[vertex.Length * 3];
			for (int i = 0, j = 0; i < vertex.Length; i++, j += 3) {
				arrVert[j] = vertex[i].x;
				arrVert[j + 1] = vertex[i].y;
				arrVert[j + 2] = vertex[i].z;
			}

			var arrSurf = new int[surface.Length * 3];
			for (int i = 0, j = 0; i < surface.Length; i++, j += 3) {
				arrSurf[j] = surface[i].a;
				arrSurf[j + 1] = surface[i].b;
				arrSurf[j + 2] = surface[i].c;
			}

			var pVert = Marshal.AllocHGlobal(arrVert.Length * sizeof(float));
			var pSurf = Marshal.AllocHGlobal(arrSurf.Length * sizeof(int));
			Marshal.Copy(arrVert, 0, pVert, arrVert.Length);
			Marshal.Copy(arrSurf, 0, pSurf, arrSurf.Length);

			model.Id = create_model(pMat4, material, pVert, vertex.Length, pSurf, surface.Length);

			Marshal.FreeHGlobal(pVert);
			Marshal.FreeHGlobal(pSurf);
		}

		public unsafe static void create_model(Model model, Color[] vertexColor, Vec3[] vertex, Surface[] surface) {
			IntPtr pMat4;
			fixed (float* p = &model.Matrix[0]) {
				pMat4 = (IntPtr)p;
			}

			var arrVert = new float[vertex.Length * 7];
			for (int i = 0, j = 0; i < vertex.Length; i++, j += 7) {
				arrVert[j] = vertex[i].x;
				arrVert[j + 1] = vertex[i].y;
				arrVert[j + 2] = vertex[i].z;
				arrVert[j + 3] = vertexColor[i].R / 255.0f;
				arrVert[j + 4] = vertexColor[i].G / 255.0f;
				arrVert[j + 5] = vertexColor[i].B / 255.0f;
				arrVert[j + 6] = vertexColor[i].A / 255.0f;
			}

			var arrSurf = new int[surface.Length * 3];
			for (int i = 0, j = 0; i < surface.Length; i++, j += 3) {
				arrSurf[j] = surface[i].a;
				arrSurf[j + 1] = surface[i].b;
				arrSurf[j + 2] = surface[i].c;
			}

			var pVert = Marshal.AllocHGlobal(arrVert.Length * sizeof(float));
			var pSurf = Marshal.AllocHGlobal(arrSurf.Length * sizeof(int));
			Marshal.Copy(arrVert, 0, pVert, arrVert.Length);
			Marshal.Copy(arrSurf, 0, pSurf, arrSurf.Length);

			model.Id = create_model_color(pMat4, pVert, vertex.Length, pSurf, surface.Length);

			Marshal.FreeHGlobal(pVert);
			Marshal.FreeHGlobal(pSurf);
		}
	}
}
