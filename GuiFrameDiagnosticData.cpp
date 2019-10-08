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
	auto maxMsIt = 
		max_element(frameDiagData.begin(), frameDiagData.end(), 
					[](FrameDiagnosticData const& a, 
					   FrameDiagnosticData const& b)->bool
					{
						return a.milliseconds < b.milliseconds;
					});
	auto maxLogicIt = 
		max_element(frameDiagData.begin(), frameDiagData.end(), 
					[](FrameDiagnosticData const& a, 
					   FrameDiagnosticData const& b)->bool
					{
						return a.logicFrames < b.logicFrames;
					});
	auto maxWastedLogicIt = 
		max_element(frameDiagData.begin(), frameDiagData.end(), 
					[](FrameDiagnosticData const& a, 
					   FrameDiagnosticData const& b)->bool
					{
						return a.wastedLogicFrames < b.wastedLogicFrames;
					});
	string strMaxMs;
	string strMaxLogic;
	string strMaxWasted;
	{
		stringstream ssMaxMs;
		ssMaxMs << "max=" << maxMsIt->milliseconds;
		strMaxMs = ssMaxMs.str();
	}
	{
		stringstream ssMaxLogic;
		ssMaxLogic << "max=" << maxLogicIt->logicFrames;
		strMaxLogic = ssMaxLogic.str();
	}
	{
		stringstream ssMaxWasted;
		ssMaxWasted << "max=" << maxWastedLogicIt->wastedLogicFrames;
		strMaxWasted = ssMaxWasted.str();
	}
	ImGui::Begin("DEBUG Frame Diagnostics");
	ImGui::PlotHistogram("ms", (float*)(&frameDiagData[0].milliseconds),
						 (int)frameDiagData.size(),
						 frameDiagDataOffset,
						 strMaxMs.c_str(),
						 FLT_MAX, FLT_MAX, ImVec2(0, 80), 
						 sizeof(FrameDiagnosticData));
	ImGui::PlotHistogram("logic ticks", (float*)(&frameDiagData[0].logicFrames),
						 (int)frameDiagData.size(),
						 frameDiagDataOffset,
						 strMaxLogic.c_str(), 
						 FLT_MAX, FLT_MAX, ImVec2(0, 80), 
						 sizeof(FrameDiagnosticData));
	ImGui::PlotHistogram("wasted logic ticks", 
						 (float*)(&frameDiagData[0].wastedLogicFrames),
						 (int)frameDiagData.size(),
						 frameDiagDataOffset,
						 strMaxWasted.c_str(), 
						 FLT_MAX, FLT_MAX, ImVec2(0, 80), 
						 sizeof(FrameDiagnosticData));
	ImGui::End();
}