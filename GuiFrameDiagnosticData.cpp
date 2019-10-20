#include "GuiFrameDiagnosticData.h"
void GuiFrameDiagnosticData::append(i32 milliseconds, u32 logicTicks, u32 wastedLogicTicks)
{
	frameDiagData[frameDiagDataOffset] = 
		{ static_cast<float>(milliseconds),
		  static_cast<float>(logicTicks), 
		  static_cast<float>(wastedLogicTicks) };
	frameDiagDataOffset = (frameDiagDataOffset + 1) % frameDiagData.size();
}
void GuiFrameDiagnosticData::draw() const
{
	OPTICK_EVENT();
	auto maxMsIt = 
		max_element(frameDiagData.begin(), frameDiagData.end(), 
					[](FrameDiagnosticData const& a, 
					   FrameDiagnosticData const& b)->bool
					{
						return a.milliseconds < b.milliseconds;
					});
	auto minMsIt = 
		max_element(frameDiagData.begin(), frameDiagData.end(), 
					[](FrameDiagnosticData const& a, 
					   FrameDiagnosticData const& b)->bool
					{
						return a.milliseconds > b.milliseconds;
					});
	float avgMs = 0;
	for (auto const& frame : frameDiagData)
	{
		avgMs += frame.milliseconds;
	}
	avgMs /= frameDiagData.size();
	auto maxLogicIt = 
		max_element(frameDiagData.begin(), frameDiagData.end(), 
					[](FrameDiagnosticData const& a, 
					   FrameDiagnosticData const& b)->bool
					{
						return a.logicFrames < b.logicFrames;
					});
	auto minLogicIt = 
		max_element(frameDiagData.begin(), frameDiagData.end(), 
					[](FrameDiagnosticData const& a, 
					   FrameDiagnosticData const& b)->bool
					{
						return a.logicFrames > b.logicFrames;
					});
	auto maxWastedLogicIt = 
		max_element(frameDiagData.begin(), frameDiagData.end(), 
					[](FrameDiagnosticData const& a, 
					   FrameDiagnosticData const& b)->bool
					{
						return a.wastedLogicFrames < b.wastedLogicFrames;
					});
	auto minWastedLogicIt = 
		max_element(frameDiagData.begin(), frameDiagData.end(), 
					[](FrameDiagnosticData const& a, 
					   FrameDiagnosticData const& b)->bool
					{
						return a.wastedLogicFrames > b.wastedLogicFrames;
					});
	string strMs;
	string strLogic;
	string strWasted;
	{
		stringstream ss;
		ss << "["<< minMsIt->milliseconds<<"," << maxMsIt->milliseconds<<"] ~"<< int(avgMs);
		strMs = ss.str();
	}
	{
		stringstream ss;
		ss << "[" << minLogicIt->logicFrames << "," << maxLogicIt->logicFrames << "]";
		strLogic = ss.str();
	}
	{
		stringstream ss;
		ss << "[" << minWastedLogicIt->wastedLogicFrames << "," << maxWastedLogicIt->wastedLogicFrames << "]";
		strWasted = ss.str();
	}
	ImGui::Begin("DEBUG Frame Diagnostics");
	ImGui::PlotHistogram("ms", (float*)(&frameDiagData[0].milliseconds),
						 (int)frameDiagData.size(),
						 frameDiagDataOffset,
						 strMs.c_str(),
						 FLT_MAX, FLT_MAX, ImVec2(0, 80), 
						 sizeof(FrameDiagnosticData));
	ImGui::PlotHistogram("logic ticks", (float*)(&frameDiagData[0].logicFrames),
						 (int)frameDiagData.size(),
						 frameDiagDataOffset,
						 strLogic.c_str(), 
						 FLT_MAX, FLT_MAX, ImVec2(0, 80), 
						 sizeof(FrameDiagnosticData));
	ImGui::PlotHistogram("wasted logic ticks", 
						 (float*)(&frameDiagData[0].wastedLogicFrames),
						 (int)frameDiagData.size(),
						 frameDiagDataOffset,
						 strWasted.c_str(), 
						 FLT_MAX, FLT_MAX, ImVec2(0, 80), 
						 sizeof(FrameDiagnosticData));
	ImGui::End();
}