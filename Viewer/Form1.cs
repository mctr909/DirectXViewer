using System;
using System.Drawing;
using System.Windows.Forms;

namespace Viewer {
	public partial class Form1 : Form {
		DirectX.Camera mCam = new DirectX.Camera() {
			x = 100,
			y = 200,
			z = 300,
			azimuth = 6.28f,
			elevetion = 3.14f
		};
		DirectX.Model mModel = new DirectX.Model();
		DirectX.Surface[] mSurface;
		double mTheta = 0.0;

		public Form1() {
			InitializeComponent();
			createSurface(114, 512);
		}

		private void timer1_Tick(object sender, EventArgs e) {
			createModel(160.0f, 160.0f, 114, 512);
			DirectX.Api.render();
			DirectX.Api.delete_model(mModel.Id);
			mTheta += Math.PI * timer1.Interval / 500.0;
			if (2 * Math.PI <= mTheta) {
				mTheta -= 2 * Math.PI;
			}
		}

		private void btnOpen_Click(object sender, EventArgs e) {
			DirectX.Api.open(320, 240, ref mCam);
			timer1.Interval = 16;
			timer1.Enabled = true;
			timer1.Start();
		}

		private void btnClose_Click(object sender, EventArgs e) {
			DirectX.Api.close();
		}

		float getValue(float x, float y) {
			var c = Math.Cos(0.1f * y + mTheta);
			return (float)(10.0f * Math.Sin(0.1f * x) * c * c * c);
		}

		void createModel(float width, float depth, int m, int n) {
			int vertexCount = m * n;

			float halfwidth = width * 0.5f;
			float halfdepth = depth * 0.5f;

			float dx = width / (m - 1);
			float dy = depth / (n - 1);

			var vertex = new Vec3[vertexCount];
			for (var j = 0; j < n; ++j) {
				for (var i = 0; i < m; ++i) {
					var x = -halfwidth + i * dx;
					var y = -halfdepth + j * dy;
					vertex[i + j * m] = new Vec3(x, y, getValue(x, y));
				}
			}

			var k = 0;
			var vertexColor = new Color[vertexCount];
			foreach (var v in vertex) {
				float gray = v.z / 30.0f + 0.5f;
				if (gray < 0) {
					gray = 0;
				}
				if (1 < gray) {
					gray = 1;
				}
				float r, g, b;
				if (gray < 0.25f) {
					r = 0;
					g = gray * 4;
					b = 1;
				} else if (gray < 0.5f) {
					r = 0;
					g = 1;
					b = 1 - (gray - 0.25f) * 4;
				} else if (gray < 0.75f) {
					r = (gray - 0.5f) * 4;
					g = 1;
					b = 0;
				} else {
					r = 1;
					g = 1 - (gray - 0.75f) * 4;
					b = 0;
				}
				vertexColor[k++] = Color.FromArgb(255, (byte)(r * 255), (byte)(g * 255), (byte)(b * 255));
			}

			DirectX.Api.create_model(mModel, vertexColor, vertex, mSurface);
		}

		void createSurface(int m, int n) {
			var faceCount = (m - 1) * (n - 1) * 2;
			mSurface = new DirectX.Surface[faceCount];
			var k = 0;
			for (var j = 0; j < n - 1; ++j) {
				for (var i = 0; i < m - 1; ++i) {
					mSurface[k] = new DirectX.Surface() {
						a = i + j * m,
						b = i + 1 + j * m,
						c = i + (j + 1) * m
					};
					mSurface[k + 1] = new DirectX.Surface() {
						a = i + (j + 1) * m,
						b = i + 1 + j * m,
						c = i + 1 + (j + 1) * m
					};
					k += 2;
				}
			}
		}
	}
}
