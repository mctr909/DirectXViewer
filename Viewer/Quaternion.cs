using System;

struct Quaternion {
	public float w, x, y, z;

	public Quaternion(float w = 1, float x = 0, float y = 0, float z = 0) {
		this.w = w;
		this.x = x;
		this.y = y;
		this.z = z;
	}

	/// <summary>回転量ベクトルによる回転</summary> ///
	public void Rotate(Vec3 omega) {
		var dw = (omega.x * x + omega.y * y + omega.z * z) * -0.5f;
		var dx = (omega.x * w + omega.z * y - omega.y * z) * 0.5f;
		var dy = (omega.y * w - omega.z * x + omega.x * z) * 0.5f;
		var dz = (omega.z * w + omega.y * x - omega.x * y) * 0.5f;
		w += dw;
		x += dx;
		y += dy;
		z += dz;
		var r = (float)Math.Sqrt(w * w + x * x + y * y + z * z);
		if (0 != r) {
			r = 1 / r;
		}
		w *= r;
		x *= r;
		y *= r;
		z *= r;
	}

	/// <summary>4x4行列へ変換</summary> ///
	public float[] ToMatrix4() {
		var ww = w * w;
		return new float[] {
			(ww + x * x) * 2 - 1, (x * y - w * z) * 2, (x * z + w * y) * 2, 0,
			(x * y + w * z) * 2, (ww + y * y) * 2 - 1, (y * z - w * x) * 2, 0,
			(x * z - w * y) * 2, (y * z + w * x) * 2, (ww + z * z) * 2 - 1, 0,
			0, 0, 0, 1
		};
	}
}
