#define NOMINMAX
#include <Novice.h>
#include <numbers>
#include <cmath>
#include <algorithm>
#include <imgui.h>
const char kWindowTitle[] = "学籍番号";

struct Matrix4x4
{
	float m[4][4];
};

struct  Spherical
{
	float radius;

	float theta;

	float phi;

};

Spherical s
{
	6.0f,
	0.0f,
	-std::numbers::pi_v<float> / 2.0f
};

struct Vector3
{
	float x;
	float y;
	float z;
};

Vector3 operator+(const Vector3& a, const Vector3& b)
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

Vector3 ToCartesian(const Spherical& sph )
{
	float rho = sph.radius * std::cos(sph.theta);
	return
	{
		rho * std::cos(sph.phi),
		sph.radius * std::sin(sph.theta),
		rho * std::sin(sph.phi)
	};

}

Spherical ToSpherical(const Vector3& p)
{
	float r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
	if (r == 0.0f)
	{
		return{ 0.0f,0.0f,0.0f };
	}

	float sinTheta = std::clamp(p.y / r, -1.0f, 1.0f);
	float phi = 0.0f;
	if (p.x != 0.0f || p.z != 0.0f)
	{
		phi = std::atan2(p.z, p.x);
	}
	return{ r, std::asin(sinTheta), phi };
}

Vector3 Normalize(const Vector3& v)
{
	float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	if (length == 0.0f)
	{
		return { 0.0f, 0.0f, 0.0f };
	}
	return { v.x / length, v.y / length, v.z / length };
}

Vector3 Normalize(const Vector3& to, const Vector3& from)
{
	Vector3 v = { to.x - from.x, to.y - from.y, to.z - from.z };
	return Normalize(v);  
}

Vector3 Cross(const Vector3& a, const Vector3& b)
{
	return
	{
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0)
	{
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		const float limit = std::numbers::pi_v<float> / 2.0f - 0.01f;
		s.radius = (std::max)(s.radius, 0.1f);
		s.theta = std::clamp(s.theta, -limit, limit);

		Vector3 offset = ToCartesian(s);
		Vector3 target = { 0.0f, 0.0f, 0.0f };
		Vector3 eye = target + offset;
		Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
		Vector3 forward = Normalize(target, eye);
		Vector3 right = Normalize(Cross(worldUp, forward));
		Vector3 up = Cross(forward, right);

		Matrix4x4 cameraMatrix
		{
			{
				{ right.x,   right.y,   right.z,   0.0f },
				{ up.x,      up.y,      up.z,      0.0f },
				{ forward.x, forward.y, forward.z, 0.0f },
				{ eye.x,     eye.y,     eye.z,     1.0f }
			}
		};


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		ImGui::Begin("Camera");

		ImGui::DragFloat("radius", &s.radius, 0.01f);
		ImGui::DragFloat("theta (rad)", &s.theta, 0.01f);
		ImGui::DragFloat("phi (rad)", &s.phi, 0.01f);

		ImGui::Text("pos: %.3f, %.3f, %.3f", eye.x, eye.y, eye.z);

		ImGui::Text("cameraMatrix");
		for (int row = 0; row < 4; ++row)
		{
			ImGui::Text("%.3f  %.3f  %.3f  %.3f",
				cameraMatrix.m[row][0], cameraMatrix.m[row][1],
				cameraMatrix.m[row][2], cameraMatrix.m[row][3]);
		}

		ImGui::End();


		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
