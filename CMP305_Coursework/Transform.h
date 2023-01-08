#pragma once


#include <DirectXMath.h>
#include "imgui/imgui.h"

using namespace DirectX;

class Transform
{
public:
	Transform() = default;

	void SetTranslation(XMFLOAT3 t) { m_Translation = t; }
	XMFLOAT3 GetTranslation() const { return m_Translation; }

	void SetScale(float s) { m_Scale = { s, s, s }; }
	void SetScale(XMFLOAT3 s) { m_Scale = s; }
	XMFLOAT3 GetScale() const { return m_Scale; }

	XMMATRIX GetMatrix() const 
	{
		XMMATRIX m = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
		m *= XMMatrixTranslation(m_Translation.x, m_Translation.y, m_Translation.z);
		return m;
	}

	void SettingsGUI()
	{
		ImGui::DragFloat3("Position", &m_Translation.x, 0.01f);
		ImGui::DragFloat3("Scale", &m_Scale.x, 0.01f);
	}


private:
	XMFLOAT3 m_Translation { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_Scale { 1.0f, 1.0f, 1.0f };
};
