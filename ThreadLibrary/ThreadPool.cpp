/*
	Author: Derek Ackworth
	Date: December 9th, 2019
	File: ThreadPool.cpp
	Purpose: ThreadPool class implementation
*/

#include <Windows.h>
#include "ThreadPool.hpp"

// Contruct thread pool based on cpu cores
ThreadPool::ThreadPool()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	numberOfThreads_ = si.dwNumberOfProcessors;
	start();
}

/*
	Construct thread pool based on input
	Input: Number of threads to create
*/
ThreadPool::ThreadPool(size_t numberOfThreads_)
{
	this->numberOfThreads_ = numberOfThreads_;
	start();
}

// Destruct thread pool
ThreadPool::~ThreadPool()
{
	{
		unique_lock<mutex> lk(wakeMtx_);
		isRunning_ = false;
	}

	wakeCond_.notify_all();

	for (thread& thread : threads_)
	{
		thread.join();
	}
}

// Start thread pool
void ThreadPool::start()
{
	isRunning_ = true;

	for (unsigned int i = 0; i < numberOfThreads_; i++)
	{
		threads_.emplace_back([=]
		{
			while (isRunning_)
			{
				{
					unique_lock<mutex> lk(wakeMtx_);
					wakeCond_.wait(lk);
				}

				while (!tasks_.empty())
				{
					function<void()> task;
					bool hasTask = false;

					{
						unique_lock<mutex> lk(taskMtx_);

						if (!tasks_.empty())
						{
							task = tasks_.front();
							tasks_.pop();
							hasTask = true;
						}
					}

					if (hasTask)
					{
						task();
					}
				}
			}
		});
	}
}

/*
	Add task to queue and notify a thread to perform
	Input: Task function
*/
void ThreadPool::performTask(function<void()> task)
{
	{
		unique_lock<mutex> lk(taskMtx_);
		tasks_.push(task);
	}

	wakeCond_.notify_one();
}
