#pragma once
#include "vkUtil.h"
#include <thread>
#include <queue>
#include <functional>
#include <mutex>

class Thread {
public:
	Thread() {
		worker = std::thread(&Thread::QueueLoop);
	}
	~Thread() {
		if (worker.joinable()) {
			Wait();
		}
	}
	void AddJob(std::function<void()> function) {
		std::lock_guard<std::mutex> lock(queueMutex);
		jobQueue.push(std::move(function));
		condition.notify_one();
	}
	void Wait() {
		std::unique_lock<std::mutex> lock(queueMutex);
		condition.wait(lock, [this] { return jobQueue.empty(); });
	}

private:
	bool destroyed = false;
	std::thread worker;
	std::queue<std::function<void()>> jobQueue;
	std::mutex queueMutex;
	std::condition_variable condition;

	void QueueLoop() {
		while (true) {
			std::function<void()> job;
			{
				std::unique_lock<std::mutex> lock(queueMutex);
				condition.wait(lock, [this] { return !jobQueue.empty() || destroyed; });
				if (destroyed) {
					break;
				}
				job = jobQueue.front();
			}
			job();
			{
				std::lock_guard<std::mutex> lock(queueMutex);
				jobQueue.pop();
				condition.notify_one();
			}
		}
	}
};

class ThreadPool {
public:
	std::vector<std::unique_ptr<Thread>> threads;

	void SetThreadCount(uint32_t count) {
		threads.clear();
		for (uint32_t i = 0; i < count; i++) {
			threads.push_back(std::make_unique<Thread>());
		}
	}

	void Wait() {
		for (auto& thread : threads) {
			thread->Wait();
		}
	}
};