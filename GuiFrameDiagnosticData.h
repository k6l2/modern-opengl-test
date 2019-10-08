#pragma once
class GuiFrameDiagnosticData
{
public:
	void append(i32 milliseconds, u32 logicTicks, u32 wastedLogicTicks);
	void draw() const;
private:
	struct FrameDiagnosticData
	{
		float milliseconds;
		float logicFrames;
		float wastedLogicFrames;
	};
private:
	vector<FrameDiagnosticData> frameDiagData =
		vector<FrameDiagnosticData>(60 * 5, { 0.f,0.f,0.f });
	int frameDiagDataOffset = 0;
};
