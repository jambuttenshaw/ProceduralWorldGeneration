#pragma once

#include "TerrainMesh.h"
#include "Transform.h"
#include "DXF.h"

struct GameObject
{
	enum class MeshType
	{
		Regular,
		Terrain
	};
	MeshType meshType;

	Transform transform;
	union
	{
		BaseMesh* regular = nullptr;
		TerrainMesh* terrain;
	} mesh;

	GameObject(TerrainMesh* mesh, XMFLOAT3 pos)
	{
		meshType = MeshType::Terrain;
		this->mesh.terrain = mesh;
		transform.SetTranslation(pos);
	}
	GameObject(BaseMesh* mesh, XMFLOAT3 pos)
	{
		meshType = MeshType::Regular;
		this->mesh.regular = mesh;
		transform.SetTranslation(pos);
	}

	void SettingsGUI()
	{
		transform.SettingsGUI();
	}

};
