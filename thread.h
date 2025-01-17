#pragma once
#include <thread>

class Thread
{
public:
	Thread() {}
	~Thread() {
		Thread::Stop();
	}
	int Start() {}
	int Stop() {
		abort_ = 1;
		if (thread_) {
			thread_->join();
			delete thread_;
			thread_ = NULL;
		}
		return 0;
	}
	virtual void  Run() = 0;
protected:
	int abort_ = 0;
	std::thread* thread_ = NULL;
};

