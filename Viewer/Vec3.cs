using System;

struct Vec3 {
	public float x, y, z;

	public Vec3(float x = 0, float y = 0, float z = 0) {
		this.x = x;
		this.y = y;
		this.z = z;
	}

	/// <summary>符号反転</summary> ///
	public static Vec3 operator -(Vec3 v) {
		return new Vec3(-v.x, -v.y, -v.z);
	}

	/// <summary>加算</summary> ///
	public static Vec3 operator +(Vec3 a, Vec3 b) {
		return new Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
	}
	/// <summary>減算</summary> ///
	public static Vec3 operator -(Vec3 a, Vec3 b) {
		return new Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
	}
	/// <summary>クロス積</summary> ///
	public static Vec3 operator *(Vec3 a, Vec3 b) {
		return new Vec3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}
	/// <summary>スカラー積</summary> ///
	public static Vec3 operator *(Vec3 v, float s) {
		return new Vec3(v.x * s, v.y * s, v.z * s);
	}
	/// <summary>スカラー積</summary> ///
	public static Vec3 operator *(float s, Vec3 v) {
		return new Vec3(v.x * s, v.y * s, v.z * s);
	}
	/// <summary>クォータニオンによる回転</summary> ///
	public static Vec3 operator *(Quaternion q, Vec3 v) {
		var ww = q.w * q.w;
		return new Vec3(
			((ww + q.x * q.x - 0.5f) * v.x + (q.x * q.y - q.w * q.z) * v.y + (q.x * q.z + q.w * q.y) * v.z) * 2,
			((q.x * q.y + q.w * q.z) * v.x + (ww + q.y * q.y - 0.5f) * v.y + (q.y * q.z - q.w * q.x) * v.z) * 2,
			((q.x * q.z - q.w * q.y) * v.x + (q.y * q.z + q.w * q.x) * v.y + (ww + q.z * q.z - 0.5f) * v.z) * 2
		);
	}
	/// <summary>スカラー積(v/s)</summary> ///
	public static Vec3 operator /(Vec3 v, float s) {
		return new Vec3(v.x / s, v.y / s, v.z / s);
	}
	/// <summary>正規化してスケーリング</summary> ///
	public static Vec3 operator |(Vec3 v, float scale) {
		var r = (float)Math.Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		if (0 != r) {
			r = scale / r;
		}
		return new Vec3(v.x * r, v.y * r, v.z * r);
	}

	/// <summary>ユークリッドノルム</summary> ///
	public float Norm() {
		return (float)Math.Sqrt(x * x + y * y + z * z);
	}
}
