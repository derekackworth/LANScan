/*
	Author: Derek Ackworth
	Date: December 9th, 2019
	File: ThreadPool.hpp
	Purpose: ThreadPool class declaration
*/

#pragma once

#include <functional>
#include <vector>
#include <thread>
#include <condition_variable>
#include <queue>

using namespace std;

class ThreadPool
{
public:
	ThreadPool();
	ThreadPool(size_t numberOfThreads_);
	~ThreadPool();

	void start();
	void performTask(function<void()> task);

private:
	size_t numberOfThreads_;
	bool isRunning_;
	vector<thread> threads_;
	condition_variable wakeCond_;
	mutex wakeMtx_;
	queue<function<void()>> tasks_;
	mutex taskMtx_;
};
