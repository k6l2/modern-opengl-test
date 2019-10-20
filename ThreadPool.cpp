#include "ThreadPool.h"
JobDataGeneric::JobDataGeneric(JobDataProcessModels const& jd)
{
	processModels = jd;
}
JobDataGeneric::JobDataGeneric(JobDataCopyDataV2f const& jd)
{
	copyDataV2f = jd;
}
JobDataGeneric::JobDataGeneric(JobDataCopyDataFloat const& jd)
{
	copyDataFloat = jd;
}
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
void ThreadPool::waitUntilAllWorkIsFinished()
{
	OPTICK_EVENT();
///	workerThreadCondition.notify_all();
	while (true)
	{
		{
			unique_lock<mutex> lock(allWorkFinishedMutex);
			allWorkFinishedCondition.wait(lock, [this]()->bool
				{
					return allWorkFinished();
				});
			// re-check condition in case of a spurious wakeup //
			if (!allWorkFinished())
			{
				continue;
			}
		}
		// once all work has finished, we can finally release control flow //
		break;
	}
}
void ThreadPool::postJob(JobDataGeneric const& job)
{
	OPTICK_EVENT();
	unique_lock<mutex> lock(workerThreadMutex);
	jobs.push(job);
	jobsPosted++;
	lock.unlock();
	workerThreadCondition.notify_one();
}
int ThreadPool::workerThreadMain(void* data)
{
	OPTICK_THREAD("ThreadPoolWorker");
	ThreadPool*const tpOwner = 
		static_cast<ThreadPool*>(data);
	JobDataGeneric currentJob(JobDataProcessModels{});
	while (true)
	{
		// keep the thread idle and only wake under certain conditions //
		{
			unique_lock<mutex> lock(tpOwner->workerThreadMutex);
			tpOwner->workerThreadCondition.wait(lock, [tpOwner]()->bool
				{
					return !tpOwner->running ||
						!tpOwner->jobs.empty();
				});
			if (!tpOwner->running)
			{
				SDL_Log("Shutting down worker thread...\n");
				break;
			}
			if (tpOwner->jobs.empty())
			{
				continue;
			}
			currentJob = tpOwner->jobs.front();
			tpOwner->jobs.pop();
		}
		// at this point, we should be hired! Now we can work~~~ //
///		SDL_Log("Hired for job!\n");
		switch (currentJob.jobTitle)
		{
		case JobTitle::PROCESS_MODELS: {
			OPTICK_EVENT("JobTitle::PROCESS_MODELS");
			for (size_t m = currentJob.processModels.modelOffset;
				 m < currentJob.processModels.modelOffset +
				currentJob.processModels.modelCount; m++)
			{
				static const float RADIANS_PER_SECOND = k10::PI * 2.f;
				const size_t meshCol = m % currentJob.processModels.numMeshCols;
				const size_t meshRow = m / currentJob.processModels.numMeshCols;
				currentJob.processModels.modelsTranslation[m] =
					currentJob.processModels.meshAabb * v2f(meshCol, meshRow);
				currentJob.processModels.modelsRadians[m] +=
					RADIANS_PER_SECOND * k10::FIXED_TIME_PER_FRAME.seconds();
				currentJob.processModels.modelsScales[m] = v2f(20, 20);
			}
		} break;
		case JobTitle::COPY_DATA_V2F: {
			OPTICK_EVENT("JobTitle::COPY_DATA_V2F");
			static const size_t stride = sizeof(v2f);
			const size_t destByteOffset =
				stride * currentJob.copyDataV2f.destinationOffset;
			const size_t dataSize = 
				stride * currentJob.copyDataV2f.elementCount;
			for (size_t c = 0; c < currentJob.copyDataV2f.elementCount; c++)
			{
				currentJob.copyDataV2f.destination[c + currentJob.copyDataV2f.destinationOffset] =
					currentJob.copyDataV2f.source[c];
			}
//			memcpy(currentJob.copyDataV2f.destination + destByteOffset,
//				currentJob.copyDataV2f.source, dataSize);
		} break;
		case JobTitle::COPY_DATA_FLOAT: {
			OPTICK_EVENT("JobTitle::COPY_DATA_FLOAT");
			static const size_t stride = sizeof(float);
			const size_t destByteOffset =
				stride * currentJob.copyDataFloat.destinationOffset;
			const size_t dataSize =
				stride * currentJob.copyDataFloat.elementCount;
			for (size_t c = 0; c < currentJob.copyDataFloat.elementCount; c++)
			{
				currentJob.copyDataFloat.destination[c + currentJob.copyDataFloat.destinationOffset] =
					currentJob.copyDataFloat.source[c];
			}
//			memcpy(currentJob.copyDataFloat.destination + destByteOffset,
//				currentJob.copyDataFloat.source, dataSize);
		} break;
		}
		unique_lock<mutex> lock(tpOwner->allWorkFinishedMutex);
		tpOwner->jobsCompleted++;
		if (tpOwner->allWorkFinished())
		{
			tpOwner->allWorkFinishedCondition.notify_all();
		}
	}
	return EXIT_SUCCESS;
}