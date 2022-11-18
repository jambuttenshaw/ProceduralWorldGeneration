#pragma once

#include <DirectXMath.h>
#include <nlohmann/json.hpp>

class SerializationHelper
{
public:
	// pure static class
	SerializationHelper() = delete;

	static nlohmann::json SerializeFloat2(const DirectX::XMFLOAT2& v);
	static nlohmann::json SerializeFloat3(const DirectX::XMFLOAT3& v);
	static nlohmann::json SerializeFloat4(const DirectX::XMFLOAT4& v);

	static void LoadFloat2FromJson(DirectX::XMFLOAT2* v, const nlohmann::json& data);
	static void LoadFloat3FromJson(DirectX::XMFLOAT3* v, const nlohmann::json& data);
	static void LoadFloat4FromJson(DirectX::XMFLOAT4* v, const nlohmann::json& data);
};