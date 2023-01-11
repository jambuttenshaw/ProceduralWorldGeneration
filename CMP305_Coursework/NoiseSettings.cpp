#include "NoiseSettings.h"

#include "SerializationHelper.h"
#include "imGUI/imgui.h"


bool SimpleNoiseSettings::SettingsGUI()
{
	bool changed = false;

	if (ImGui::TreeNode("Basic Settings"))
	{
		changed |= ImGui::DragFloat("Elevation", &Elevation, 0.01f);
		changed |= ImGui::DragFloat("Frequency", &Frequency, 0.01f);
		changed |= ImGui::DragFloat("Vertical Shift", &VerticalShift, 0.01f);
		changed |= ImGui::DragFloat2("Offset", &Offset.x, 0.001f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Fractal Settings"))
	{
		changed |= ImGui::SliderInt("Octaves", &Octaves, 1, 10);
		changed |= ImGui::DragFloat("Persistence", &Persistence, 0.001f);
		changed |= ImGui::DragFloat("Lacunarity", &Lacunarity, 0.01f);

		ImGui::TreePop();
	}

	return changed;
}

nlohmann::json SimpleNoiseSettings::Serialize() const
{
	nlohmann::json serialized;

	serialized["elevation"] = Elevation;
	serialized["frequency"] = Frequency;
	serialized["verticalShift"] = VerticalShift;
	serialized["offsetX"] = Offset.x;
	serialized["offsetY"] = Offset.y;

	serialized["octaves"] = Octaves;
	serialized["persistence"] = Persistence;
	serialized["lacunarity"] = Lacunarity;

	return serialized;
}

void SimpleNoiseSettings::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("elevation")) Elevation = data["elevation"];
	if (data.contains("frequency")) Frequency = data["frequency"];
	if (data.contains("verticalShift")) VerticalShift = data["verticalShift"];
	if (data.contains("offsetX")) Offset.x = data["offsetX"];
	if (data.contains("offsetY")) Offset.y = data["offsetY"];

	if (data.contains("octaves")) Octaves = data["octaves"];
	if (data.contains("persistence")) Persistence = data["persistence"];
	if (data.contains("lacunarity")) Lacunarity = data["lacunarity"];
}




bool RidgeNoiseSettings::SettingsGUI()
{
	bool changed = false;

	if (ImGui::TreeNode("Basic Settings"))
	{
		changed |= ImGui::DragFloat("Elevation", &Elevation, 0.01f);
		changed |= ImGui::DragFloat("Frequency", &Frequency, 0.01f);
		changed |= ImGui::DragFloat("Vertical Shift", &VerticalShift, 0.01f);
		changed |= ImGui::DragFloat2("Offset", &Offset.x, 0.001f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Fractal Settings"))
	{
		changed |= ImGui::SliderInt("Octaves", &Octaves, 1, 10);
		changed |= ImGui::DragFloat("Persistence", &Persistence, 0.001f);
		changed |= ImGui::DragFloat("Lacunarity", &Lacunarity, 0.01f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Ridge Settings"))
	{
		changed |= ImGui::DragFloat("Power", &Power, 0.01f);
		changed |= ImGui::DragFloat("Gain", &Gain, 0.01f);
		changed |= ImGui::SliderFloat("Peak Smoothing", &PeakSmoothing, 0.0f, 0.5f);

		ImGui::TreePop();
	}

	return changed;
}

nlohmann::json RidgeNoiseSettings::Serialize() const
{
	nlohmann::json serialized;

	serialized["elevation"] = Elevation;
	serialized["frequency"] = Frequency;
	serialized["verticalShift"] = VerticalShift;
	serialized["offsetX"] = Offset.x;
	serialized["offsetY"] = Offset.y;

	serialized["octaves"] = Octaves;
	serialized["persistence"] = Persistence;
	serialized["lacunarity"] = Lacunarity;

	serialized["power"] = Power;
	serialized["gain"] = Gain;
	serialized["peakSmoothing"] = PeakSmoothing;

	return serialized;
}

void RidgeNoiseSettings::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("elevation")) Elevation = data["elevation"];
	if (data.contains("frequency")) Frequency = data["frequency"];
	if (data.contains("verticalShift")) VerticalShift = data["verticalShift"];
	if (data.contains("offsetX")) Offset.x = data["offsetX"];
	if (data.contains("offsetY")) Offset.y = data["offsetY"];

	if (data.contains("octaves")) Octaves = data["octaves"];
	if (data.contains("persistence")) Persistence = data["persistence"];
	if (data.contains("lacunarity")) Lacunarity = data["lacunarity"];

	if (data.contains("power")) Power = data["power"];
	if (data.contains("gain")) Gain = data["gain"];
	if (data.contains("peakSmoothing")) PeakSmoothing = data["peakSmoothing"];
}




bool WarpedSimpleNoiseSettings::SettingsGUI()
{
	bool changed = false;

	if (ImGui::TreeNode("Noise Settings"))
	{
		changed |= NoiseSettings.SettingsGUI();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Warp Settings"))
	{
		changed |= WarpSettings.SettingsGUI();
		ImGui::TreePop();
	}

	return changed;
}

nlohmann::json WarpedSimpleNoiseSettings::Serialize() const
{
	nlohmann::json serialized;

	serialized["noiseSettings"] = NoiseSettings.Serialize();
	serialized["warpSettings"] = WarpSettings.Serialize();

	return serialized;
}

void WarpedSimpleNoiseSettings::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("noiseSettings")) NoiseSettings.LoadFromJson(data["noiseSettings"]);
	if (data.contains("warpSettings")) WarpSettings.LoadFromJson(data["warpSettings"]);
}




bool TerrainNoiseSettings::SettingsGUI()
{
	bool changed = false;

	if (ImGui::TreeNode("Continent Settings"))
	{
		changed |= ContinentSettings.SettingsGUI();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Mountain Settings"))
	{
		changed |= MountainSettings.SettingsGUI();
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Ocean Settings"))
	{
		changed |= ImGui::DragFloat("Ocean Depth Multiplier", &OceanDepthMultiplier, 0.01f);
		changed |= ImGui::DragFloat("Ocean Floor Depth", &OceanFloorDepth, 0.01f);
		changed |= ImGui::SliderFloat("Ocean Floor Smoothing", &OceanFloorSmoothing, 0.0f, 5.0f);
		changed |= ImGui::DragFloat("Mountain Blend", &MountainBlend, 0.01f);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Warp Settings"))
	{
		changed |= WarpSettings.SettingsGUI();
		ImGui::TreePop();
	}

	return changed;
}

nlohmann::json TerrainNoiseSettings::Serialize() const
{
	nlohmann::json serialized;

	serialized["continentSettings"] = ContinentSettings.Serialize();
	serialized["mountainSettings"] = MountainSettings.Serialize();
	serialized["warpSettings"] = WarpSettings.Serialize();

	serialized["oceanDepthMultiplier"] = OceanDepthMultiplier;
	serialized["oceanFloorDepth"] = OceanFloorDepth;
	serialized["oceanFloorSmoothing"] = OceanFloorSmoothing;
	serialized["mountainBlend"] = MountainBlend;

	return serialized;
}

void TerrainNoiseSettings::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("continentSettings")) ContinentSettings.LoadFromJson(data["continentSettings"]);
	if (data.contains("mountainSettings")) MountainSettings.LoadFromJson(data["mountainSettings"]);
	if (data.contains("warpSettings")) WarpSettings.LoadFromJson(data["warpSettings"]);

	if (data.contains("oceanDepthMultiplier")) OceanDepthMultiplier = data["oceanDepthMultiplier"];
	if (data.contains("oceanFloorDepth")) OceanFloorDepth = data["oceanFloorDepth"];
	if (data.contains("oceanFloorSmoothing")) OceanFloorSmoothing = data["oceanFloorSmoothing"];
	if (data.contains("mountainBlend")) MountainBlend = data["mountainBlend"];
}
