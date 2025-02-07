#pragma once

#include <vector>
#include <string>

#include "reproc++/run.hpp"


// helper class to render audio waves
class OFS_Waveform
{
	bool generating = false;
public:
	std::vector<float> SamplesHigh;
	std::vector<float> SamplesMid;
	std::vector<float> SamplesLow;

	float MidMax = 0.f;
	float LowMax = 0.f;

	inline bool BusyGenerating() noexcept { return generating; }

	bool LoadMP3(const std::string& path) noexcept;
	bool GenerateMP3(const std::string& ffmpegPath, const std::string& videoPath, const std::string& output) noexcept;
	
	inline void Clear() noexcept {
		SamplesLow.clear();
		SamplesMid.clear();
		SamplesHigh.clear();
	}

	inline size_t SampleCount() const noexcept {
		return SamplesHigh.size(); // all have the same size
	}
};