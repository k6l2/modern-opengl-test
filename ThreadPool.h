// Job Thread Pool
// directed by SeanMiddleditch's suggested approach in:
//	https://www.gamedev.net/forums/topic/661304-threads-with-sdl-within-c-classes/
// based on: https://stackoverflow.com/a/51400041/4526664
// w/ memory access clarification from: 
//	https://stackoverflow.com/a/30759329/4526664
#pragma once
enum class JobTitle : u8
{
	PROCESS_MODELS,
	COPY_DATA_V2F,
	COPY_DATA_FLOAT
};
struct JobDataProcessModels
{
	JobTitle jobTitle;
	v2f* modelsTranslation;
	float* modelsRadians;
	v2f* modelsScales;
	size_t modelCount;
	size_t modelOffset;
	v2f meshAabb;
	size_t numMeshCols;
};
struct JobDataCopyDataV2f
{
	JobTitle jobTitle;
	v2f* destination;
	v2f const* source;
	size_t elementCount;
	size_t destinationOffset;
};
struct JobDataCopyDataFloat
{
	JobTitle jobTitle;
	float* destination;
	float const* source;
	size_t elementCount;
	size_t destinationOffset;
};
union JobDataGeneric
{
	JobDataGeneric(JobDataProcessModels const& jd);
	JobDataGeneric(JobDataCopyDataV2f const& jd);
	JobDataGeneric(JobDataCopyDataFloat const& jd);
	JobTitle jobTitle;
	JobDataProcessModels processModels;
	JobDataCopyDataV2f copyDataV2f;
	JobDataCopyDataFloat copyDataFloat;
};
/// struct JobDataUpdateModelInstances
/// {
/// 	v2f* vbModelTranslations;
/// 	float* vbModelRadians;
/// 	v2f* vbModelScales;
/// 	v2f const* modelTranslations;
/// 	float const* modelRadians;
/// 	v2f const* modelScales;
/// 	size_t modelCount;
/// 	size_t instanceBufferOffset;
/// };
//struct JobDataBatchModels
//{
//	v2f* modelsTranslation;
//	float* modelsRadians;
//	v2f* modelsScales;
//	size_t modelCount;
//	size_t modelOffset;
//	u16 meshId;
//	class InstancedMeshCache* imc;
//};
class ThreadPool
{
public:
	void create(size_t threadCount);
	void destroy();
	void postJob(JobDataGeneric const& job);
	void waitUntilAllWorkIsFinished();
private:
	bool allWorkFinished();
	static int workerThreadMain(void* data);
private:
	vector<SDL_Thread*> threads;
	atomic<bool> running = true;
	condition_variable workerThreadCondition;
	condition_variable allWorkFinishedCondition;
	mutex workerThreadMutex;
	mutex allWorkFinishedMutex;
	queue<JobDataGeneric> jobs;
	atomic<size_t> jobsPosted    = 0;
	atomic<size_t> jobsCompleted = 0;
};
