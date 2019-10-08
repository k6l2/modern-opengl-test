// Job Thread Pool
// directed by SeanMiddleditch's suggested approach in:
//	https://www.gamedev.net/forums/topic/661304-threads-with-sdl-within-c-classes/
// based on: https://stackoverflow.com/a/51400041/4526664
// w/ memory access clarification from: 
//	https://stackoverflow.com/a/30759329/4526664
#pragma once
struct JobDataProcessModels
{
	v2f* modelsTranslation;
	float* modelsRadians;
	size_t modelCount;
	size_t modelOffset;
	v2f meshAabb;
	size_t numMeshCols;
};
class ThreadPool
{
public:
	void create(size_t threadCount);
	void destroy();
	void postJob(JobDataProcessModels const& job);
	bool allWorkFinished();
private:
	static int workerThreadMain(void* data);
private:
	vector<SDL_Thread*> threads;
	atomic<bool> running = true;
	condition_variable workerThreadCondition;
	mutex workerThreadMutex;
	queue<JobDataProcessModels> jobsProcessModels;
	atomic<size_t> jobsPosted    = 0;
	atomic<size_t> jobsCompleted = 0;
};
