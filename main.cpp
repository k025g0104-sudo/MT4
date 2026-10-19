#define NOMINMAX
#include <Novice.h>
#include <numbers>
#include <cmath>
#include <algorithm>
#include <imgui.h>
#include <chrono>
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

struct Vector2
{
	float x;
	float y;
};

Vector3 camera =
{
	0.0f, 0.0f, 6.0f
};

float t = 0.1f;
float speed = 6.0f;

Vector3 operator-(const Vector3& a, const Vector3& b)
{
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

Vector3 operator+(const Vector3& a, const Vector3& b)
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

Vector3& operator+=(Vector3& a, const Vector3& b)
{
	a.x += b.x; a.y += b.y; a.z += b.z;
	return a;
}

Vector3 operator*(float k, const Vector3& v)
{
	return { k * v.x, k * v.y, k * v.z };
}

Vector2 operator-(const Vector2& a, const Vector2& b)
{
	return { a.x - b.x, a.y - b.y };
}

Vector2 operator*(float k, const Vector2& v)
{
	return { k * v.x, k * v.y };
}

Vector2& operator+=(Vector2& a, const Vector2& b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

Vector3 ToCartesian(const Spherical& sph)
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

struct Quaternion
{
	float x;
	float y;
	float z;
	float w;

};

Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs);

Quaternion IdentityQuaternion();

Quaternion Conjugate(const Quaternion& quaternion);

float Norm(const Quaternion& quaternion);

Quaternion Normalize(const Quaternion& quaternion);

Quaternion Inverse(const Quaternion& quaternion);


Quaternion Multiply(const Quaternion& lhs, const Quaternion& rhs)
{
	return
	{
		lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x,
		lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w,
		lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
	};
}

Quaternion IdentityQuaternion()
{
	return { 0.0f, 0.0f, 0.0f, 1.0f };
}

Quaternion Conjugate(const Quaternion& quaternion)
{
	return { -quaternion.x, -quaternion.y, -quaternion.z, quaternion.w };
}

float Norm(const Quaternion& quaternion)
{
	return std::sqrt(
		quaternion.x * quaternion.x +
		quaternion.y * quaternion.y +
		quaternion.z * quaternion.z +
		quaternion.w * quaternion.w);
}

Quaternion Normalize(const Quaternion& quaternion)
{
	float norm = Norm(quaternion);
	if (norm == 0.0f)
	{
		return IdentityQuaternion();
	}
	return { quaternion.x / norm, quaternion.y / norm, quaternion.z / norm, quaternion.w / norm };
}

Quaternion Inverse(const Quaternion& quaternion)
{
	float norm = Norm(quaternion);
	float normSq = norm * norm;
	if (normSq == 0.0f)
	{
		return IdentityQuaternion();
	}
	Quaternion conj = Conjugate(quaternion);
	return { conj.x / normSq, conj.y / normSq, conj.z / normSq, conj.w / normSq };
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	auto prevTime = std::chrono::steady_clock::now();

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	Vector2 posA = { 0.0f, 0.0f };
	const int radiusA = 12;
	Vector2 posB = { 640.0f, 360.0f };
	const int radiusB = 20;

	const float fixedDeltaTime = 1.0f / 60.0f;

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0)
	{
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<float> elapsed = now - prevTime;
		float deltaTime = elapsed.count();

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
		Vector3 goal = target + offset;
		t = 1.0f - std::exp(-speed * deltaTime);

		camera += t * (goal - camera);

		Vector3 diff = goal - camera;
		float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
		if (distance < 0.001f)
		{
			camera = goal;
		}

		Vector3 worldUp = { 0.0f, 1.0f, 0.0f };
		Vector3 forward = Normalize(target, camera);
		Vector3 right = Normalize(Cross(worldUp, forward));
		Vector3 up = Cross(forward, right);

		Matrix4x4 cameraMatrix
		{
			{
				{ right.x,   right.y,   right.z,   0.0f },
				{ up.x,      up.y,      up.z,      0.0f },
				{ forward.x, forward.y, forward.z, 0.0f },
				{ camera.x,  camera.y,  camera.z,  1.0f }
			}
		};

		
		int mouseX = 0;
		int mouseY = 0;
		Novice::GetMousePosition(&mouseX, &mouseY);
		posA = { static_cast<float>(mouseX), static_cast<float>(mouseY) };

		posB += (speed * fixedDeltaTime) * (posA - posB);

		Quaternion q1 = { 2.0f, 3.0f, 4.0f, 1.0f };
		Quaternion q2 = { 1.0f, 3.0f, 5.0f, 2.0f };
		Quaternion identity = IdentityQuaternion();
		Quaternion conj = Conjugate(q1);
		Quaternion inv = Inverse(q1);
		Quaternion normal = Normalize(q1);
		Quaternion mul1 = Multiply(q1, q2);
		Quaternion mul2 = Multiply(q2, q1);
		float norm = Norm(q1);

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		Novice::DrawLine(static_cast<int>(posA.x), static_cast<int>(posA.y),static_cast<int>(posB.x), static_cast<int>(posB.y),WHITE);
		Novice::DrawEllipse(static_cast<int>(posB.x), static_cast<int>(posB.y),radiusB, radiusB, 0.0f, GREEN, kFillModeSolid);
		Novice::DrawEllipse(static_cast<int>(posA.x), static_cast<int>(posA.y),radiusA, radiusA, 0.0f, RED, kFillModeSolid);

		ImGui::Begin("Camera");

		ImGui::DragFloat("radius", &s.radius, 0.01f);
		ImGui::DragFloat("theta (rad)", &s.theta, 0.01f);
		ImGui::DragFloat("phi (rad)", &s.phi, 0.01f);
		ImGui::SliderFloat("speed", &speed, 0.0f, 60.0f);
		ImGui::Text("deltaTime: %.4f", deltaTime);
		ImGui::Text("goal: %.3f, %.3f, %.3f", goal.x, goal.y, goal.z);
		ImGui::Text("pos: %.3f, %.3f, %.3f", camera.x, camera.y, camera.z);

		ImGui::Text("cameraMatrix");
		for (int row = 0; row < 4; ++row)
		{
			ImGui::Text("%.3f  %.3f  %.3f  %.3f",
				cameraMatrix.m[row][0], cameraMatrix.m[row][1],
				cameraMatrix.m[row][2], cameraMatrix.m[row][3]);
		}

		ImGui::Text("circle A: %.1f, %.1f", posA.x, posA.y);
		ImGui::Text("circle B: %.1f, %.1f", posB.x, posB.y);

		ImGui::End();

		ImGui::Begin("Quaternion");
		ImGui::Text("%.2f %.2f %.2f %.2f : Identity", identity.x, identity.y, identity.z, identity.w);
		ImGui::Text("%.2f %.2f %.2f %.2f : Conjugate", conj.x, conj.y, conj.z, conj.w);
		ImGui::Text("%.2f %.2f %.2f %.2f : Inverse", inv.x, inv.y, inv.z, inv.w);
		ImGui::Text("%.2f %.2f %.2f %.2f : Normalize", normal.x, normal.y, normal.z, normal.w);
		ImGui::Text("%.2f %.2f %.2f %.2f : Multiply(q1, q2)", mul1.x, mul1.y, mul1.z, mul1.w);
		ImGui::Text("%.2f %.2f %.2f %.2f : Multiply(q2, q1)", mul2.x, mul2.y, mul2.z, mul2.w);
		ImGui::Text("%.2f : Norm", norm);
		ImGui::End();

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		prevTime = now;

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}