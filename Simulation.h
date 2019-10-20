#pragma once
class Simulation
{
public:
	enum ComponentFlags
	{
		COMPONENT_POSITION = 1<<0,
		COMPONENT_VELOCITY = 1<<1,
		COMPONENT_SCALE    = 1<<3,
		COMPONENT_RADIANS  = 1<<4
	};
	// Even though ObjectType=>ComponentFlags, we still need to know what type
	//	of object this is so we can draw the correct sprite?..
	//	Or... maybe there can be separate sprite components~~~
	// [ObjectType][ComponentFlags][RID (20-bits = 1048576 possibilities)]
	using ObjectId = u64;
private:
	vector<ObjectId> objectIds;
	// COMPONENTS //
	vector<v2f> position;
	vector<v2f> velocity;
	vector<v2f> scale;
	vector<float> radians;
};
