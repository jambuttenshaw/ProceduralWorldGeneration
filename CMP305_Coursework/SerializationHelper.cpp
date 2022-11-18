#include "SerializationHelper.h"

nlohmann::json SerializationHelper::SerializeFloat2(const DirectX::XMFLOAT2& v)
{
	nlohmann::json serialized;
	serialized["x"] = v.x;
	serialized["y"] = v.y;
	return serialized;
}

nlohmann::json SerializationHelper::SerializeFloat3(const DirectX::XMFLOAT3& v)
{
	nlohmann::json serialized;
	serialized["x"] = v.x;
	serialized["y"] = v.y;
	serialized["z"] = v.z;
	return serialized;
}

nlohmann::json SerializationHelper::SerializeFloat4(const DirectX::XMFLOAT4& v)
{
	nlohmann::json serialized;
	serialized["x"] = v.x;
	serialized["y"] = v.y;
	serialized["z"] = v.z;
	serialized["w"] = v.w;
	return serialized;
}

void SerializationHelper::LoadFloat2FromJson(DirectX::XMFLOAT2* v, const nlohmann::json& data)
{
	if (data.contains("x")) v->x = data["x"];
	if (data.contains("y")) v->y = data["y"];
}

void SerializationHelper::LoadFloat3FromJson(DirectX::XMFLOAT3* v, const nlohmann::json& data)
{
	if (data.contains("x")) v->x = data["x"];
	if (data.contains("y")) v->y = data["y"];
	if (data.contains("z")) v->z = data["z"];
}

void SerializationHelper::LoadFloat4FromJson(DirectX::XMFLOAT4* v, const nlohmann::json& data)
{
	if (data.contains("x")) v->x = data["x"];
	if (data.contains("y")) v->y = data["y"];
	if (data.contains("z")) v->z = data["z"];
	if (data.contains("w")) v->w = data["w"];
}
