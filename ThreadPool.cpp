#include "ThreadPool.h"
void ThreadPool::create(size_t threadCount)
{
	threads.resize(threadCount);
	for (size_t t = 0; t < threadCount; t++)
	{
		threads[t] =
			SDL_CreateThread(workerThreadMain, "ThreadPoolWorker", this);
	}
}
void ThreadPool::destroy()
{
	unique_lock<mutex> lock(workerThreadMutex);
	running = false;
	lock.unlock();
	workerThreadCondition.notify_all();
	for (size_t t = 0; t < threads.size(); t++)
	{
		SDL_WaitThread(threads[t], nullptr);
	}
}
bool ThreadPool::allWorkFinished()
{
	return jobsCompleted == jobsPosted;
}
void ThreadPool::postJob(JobDataProcessModels const& job)
{
	unique_lock<mutex> lock(workerThreadMutex);
	jobsProcessModels.push(job);
	jobsPosted++;
	lock.unlock();
	workerThreadCondition.notify_one();
}
int ThreadPool::workerThreadMain(void* data)
{
	ThreadPool*const tpOwner = 
		static_cast<ThreadPool*>(data);
	JobDataProcessModels currentJobProcessModels;
	while (true)
	{
		// keep the thread idle and only wake under certain conditions //
		{
			unique_lock<mutex> lock(tpOwner->workerThreadMutex);
			tpOwner->workerThreadCondition.wait(lock, [tpOwner]()->bool
				{
					return !tpOwner->running ||
						!tpOwner->jobsProcessModels.empty();
				});
			if (!tpOwner->running)
			{
				SDL_Log("Shutting down worker thread...\n");
				break;
			}
			if (tpOwner->jobsProcessModels.empty())
			{
				continue;
			}
			currentJobProcessModels = tpOwner->jobsProcessModels.front();
			tpOwner->jobsProcessModels.pop();
		}
		// at this point, we should be hired! Now we can work~~~ //
///		SDL_Log("Hired for job!\n");
		{
			for (size_t m = currentJobProcessModels.modelOffset; 
				 m < currentJobProcessModels.modelOffset + 
						currentJobProcessModels.modelCount; m++)
			{
				static const float RADIANS_PER_SECOND = k10::PI * 2.f;
				const size_t meshCol = m % currentJobProcessModels.numMeshCols;
				const size_t meshRow = m / currentJobProcessModels.numMeshCols;
				currentJobProcessModels.modelsTranslation[m] =
					currentJobProcessModels.meshAabb * v2f(meshCol, meshRow);
				currentJobProcessModels.modelsRadians[m] +=
					RADIANS_PER_SECOND * k10::FIXED_TIME_PER_FRAME.seconds();
				currentJobProcessModels.modelsScales[m] = v2f(20, 20);
			}
		}
		tpOwner->jobsCompleted++;
	}
	return EXIT_SUCCESS;
}