#pragma once
#include "Defines.hpp"
#include "Vector.hpp"
#include <xmmintrin.h>
#include <d3d9.h>
#include "globals.hpp"
#include <emmintrin.h>

namespace Game
{
	uint64_t GamerProfile()
	{
		return (driver::Read<uint64_t>(__ROL8__(Base + 0x653DF88 + 0x62C3AB5D73DB1EE6i64, 40) + 0x2ABB4A126D979937i64) - 43i64) ^ 0x149437F025AE55E3i64;
	}
	uint64_t GameManager()
	{
		return (__ROL8__(driver::Read<uint64_t>(Base + 0x6511860), 0x22) ^ 0x7A5E5EE36682CFD2i64) - 0x5F;
	}
	uintptr_t GetCamera()
	{
		uintptr_t cameraManager = driver::Read<uintptr_t>(Base + 0x63FF2C0);
		cameraManager = driver::Read<uintptr_t>(cameraManager + 0x40);
		cameraManager = driver::Read<uintptr_t>(cameraManager + 0x0);
		return cameraManager;
	}
	static std::pair<uint64_t, uint32_t> EntityManager(const uint64_t game_manager)
	{
		const auto decryption = [&game_manager](const uint32_t offset = 0) -> uint64_t
		{
			const auto temp = driver::Read <uint64_t>(game_manager + offset + 0x50);
			return (temp ^ 0xF7FF10FCCEE0BED8);
		};

		return { decryption(), static_cast<uint32_t>(decryption(8) & 0x3FFFFFFF) };
	}
	static uint64_t Skeleton(const uint64_t pawn)
	{
		return (__ROL8__(driver::Read<uint64_t>(pawn + 0xA90), 52) ^ 0x5Di64) - 102;
	}
	uint64_t Pawn(uint64_t address)
	{
		return __ROL8__(__ROL8__(driver::Read<uint64_t>(address + 0x38), 36) ^ 0xE3D46D76600D4EB9, 11);
	}
	uint64_t component(uint64_t pawn)
	{
		return (driver::Read<uint64_t>(pawn + 0x18));
	}
	float Health(uintptr_t pawn)
	{
		uintptr_t entity_actor = component(pawn);
		uintptr_t entity_info = driver::Read<uintptr_t>(entity_actor + 0xD8);
		uintptr_t main_component = driver::Read<uintptr_t>(entity_info + 0x8);
		return driver::Read<int>(main_component + 0x1BC);
	}
	vec3 GetPos(uint64_t entPawn)
	{
		entPawn = driver::Read<uint64_t>(entPawn + 0x18);
		return driver::Read<vec3>(entPawn + 0x50);
	}
	void TransformsCalculation(__int64 pBones, __m128* ResultPosition, __m128* BoneInfo)
	{
		__m128 v5; // xmm2
		__m128 v6; // xmm3
		__m128 v7; // xmm0
		__m128 v8; // xmm4
		__m128 v9; // xmm1

		__m128 v10 = { 0.500f, 0.500f, 0.500f, 0.500f };
		__m128 v11 = { 2.000f, 2.000f, 2.000f, 0.000f };

		__m128 v12 = driver::Read<__m128>(pBones);
		__m128 v13 = driver::Read<__m128>(pBones + 0x10);

		v5 = v13;

		v6 = _mm_mul_ps(*(__m128*)BoneInfo, v5);
		v6.m128_f32[0] = v6.m128_f32[0]
			+ (float)(_mm_cvtss_f32(_mm_shuffle_ps(v6, v6, 0x55)) + _mm_cvtss_f32(_mm_shuffle_ps(v6, v6, 0xAA)));
		v7 = _mm_shuffle_ps(v13, v5, 0xFF);
		v8 = _mm_sub_ps(
			_mm_mul_ps(_mm_shuffle_ps(v5, v5, 0xD2), *(__m128*)BoneInfo),
			_mm_mul_ps(_mm_shuffle_ps(*(__m128*)BoneInfo, *(__m128*)BoneInfo, 0xD2), v5));
		v9 = _mm_shuffle_ps(v12, v12, 0x93);
		v9.m128_f32[0] = 0.0;
		*(__m128*)ResultPosition = _mm_add_ps(
			_mm_shuffle_ps(v9, v9, 0x39),
			_mm_mul_ps(
				_mm_add_ps(
					_mm_add_ps(
						_mm_mul_ps(_mm_shuffle_ps(v8, v8, 0xD2), v7),
						_mm_mul_ps(_mm_shuffle_ps(v6, v6, 0), v5)),
					_mm_mul_ps(
						_mm_sub_ps(_mm_mul_ps(v7, v7), (__m128)v10),
						*(__m128*)BoneInfo)),
				(__m128)v11));
	}
	vec3 GetBoneLocationByIndex(uintptr_t player, int index)
	{
		__m128 arrResultPosition;
		uintptr_t pBonesChain1 = Game::Skeleton(player);
		uintptr_t pBonesChain2 = driver::Read<uintptr_t>(pBonesChain1);
		uintptr_t pBones = driver::Read<uintptr_t>(pBonesChain2 + 0x238);
		uintptr_t pBonesData = driver::Read<uintptr_t>(pBones + 0x58);
		__m128 pBoneInfo = driver::Read<__m128>((0x20 * index) + pBonesData);
		TransformsCalculation(pBones, &arrResultPosition, &pBoneInfo);
		return { arrResultPosition.m128_f32[0], arrResultPosition.m128_f32[1], arrResultPosition.m128_f32[2] };
	}
	vec3 DecryptCamera(__m128i address) // updated
	{
		__m128i vector = _mm_load_si128(&address);
		vector.m128i_u64[0] = ((vector.m128i_u64[0] - 0x64) ^ 0x7399e7a77eda71a7) - 0x77;
		vector.m128i_u64[1] = ((vector.m128i_u64[1] - 0x64) ^ 0x7399e7a77eda71a7) - 0x77;
		return *reinterpret_cast<vec3*>(&vector);
	}
	static bool WorldToScreen(vec3 world, vec2& output)
	{
		vec3 temp = world - DecryptCamera(driver::Read<__m128i>(globals.cam + 0x1C0)); // translation
		float x, y, z = { };
		x = temp.dot(DecryptCamera(driver::Read<__m128i>(globals.cam + 0x190))); //right
		y = temp.dot(DecryptCamera(driver::Read<__m128i>(globals.cam + 0x1A0))); // up
		z = temp.dot(DecryptCamera(driver::Read<__m128i>(globals.cam + 0x1B0)) * -1.f); // forward

		int width = GetSystemMetrics(SM_CXSCREEN);
		int height = GetSystemMetrics(SM_CYSCREEN);

		output.x = (width / 2.f) * (1.f + x / -driver::Read<float>(globals.cam + 0x360) / z); // fovx
		output.y = (height / 2.f) * (1.f - y / -driver::Read<float>(globals.cam + 0x364) / z); // fovy

		if (output.x < 0.0f || output.x > width)
			return false;
		return z >= 1.0f;
	}
	uint64_t ReplicationInfo(uint64_t controller)
	{
		return __ROL8__(__ROL8__(driver::Read<uint64_t>(controller + 0x70), 0x1C) - 0x4Ci64, 0x27);
	}
	std::string name(uintptr_t entity)
	{
		uintptr_t replicationInfo = Game::ReplicationInfo(entity);
		return driver::ReadUnicode(driver::Read<uintptr_t>(replicationInfo + 0x328));
	}
	static uint32_t get_team_id(const uint64_t replicationinfo)
	{
		uint64_t rax = 0ull, rbx = 0ull, rcx = 0ull, rdx = 0ull, rdi = 0ull, rsi = 0ull, r8 = 0ull, r9 = 0ull, r10 = 0ull, r11 = 0ull, r12 = 0ull, r13 = 0ull, r14 = 0ull, r15 = 0ull, rbp = replicationinfo;
		rax = driver::Read<uint64_t>(rbp + 0x70);
		rax = _rotl64(rax, 0x25);
		rax += 0x14455B1E10BCF6B1i64;
		rax = _rotl64(rax, 0x37);
		rax = driver::Read<uint64_t>(rax + 0x84);
		rax -= 0x7BB86DD6;
		rax ^= 0x63;
		rax -= 0x43;
		return rax;
	}
	const char* OperatorNames[28][6] =
	{
		// Credits Dude on UC for opnames. 
		{"AI","SMOKE","MUTE","SLEDGE","THATCHER"}, //0
		{"RECRUIT","CASTLE","ASH","PULSE","THERMITE"}, //1
		{"","DOC","ROOK","TWITCH","MONTAGNE"}, //2
		{"","GLAZ","FUZE","KAPKAN","TACHANKA"}, //3
		{"","BLITZ","IQ","JAGER","BANDIT"}, //4
		{"","BUCK","FROST"}, //5
		{"","BLACKBEARD","VALKYRIE"}, //6
		{"","CAPITAO","CAVEIRA"}, //7
		{"","HIBANA","ECHO"}, //8
		{"","JACKAL","MIRA"}, //9
		{"","YING","LESION"}, //10
		{"","ELA","ZOFIA"}, //11
		{"","DOKKAEBI","VIGIL"}, //12
		{"","","LION","FINKA"}, //13
		{"","MAESTRO","ALIBI"}, //14
		{"","MAVERICK","CLASH"}, //15
		{"","NOMAD","KAID", ""}, //16
		{"","MOZZIE","GRIDLOCK"}, //17
		{"","NOKK"}, //18
		{"","WARDEN"}, //19
		{"","GOYO"}, //20
		{"","AMARU"}, //21
		{"","KALI","WAMAI", "ACE", "ARUNI", "OSA"}, //22
		{"","ORYX", "FLORES"}, //23
		{"","IANA"}, //24
		{"","MELUSI"}, //25
		{"ZERO"}, //26
		{"THUNDERBIRD"}, //26
	};
	auto OpName(uintptr_t player) -> const char*
	{
		uintptr_t replicationInfo = Game::ReplicationInfo(player);
		auto ctu = driver::Read<uint8_t>(replicationInfo + 0x1C0);
		auto op = driver::Read<uint8_t>(replicationInfo + 0x1C1);
		return OperatorNames[ctu][op];
	}
};



