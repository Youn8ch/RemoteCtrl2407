#pragma once
#include "Log.h"
#include <atomic>
#include <vector>
#include <mutex>
#include "pch.h"

class ThreadFuncBase {};
typedef int (ThreadFuncBase::* FUNCTYPE)();
class ThreadWorker
{

public:
	ThreadWorker() :thiz(NULL), func(NULL) {};
	ThreadWorker(void* obj, FUNCTYPE f) {
		thiz = (ThreadFuncBase*)obj;
		func = f;
	}
	ThreadWorker(const ThreadWorker& worker) {
		thiz = worker.thiz;
		func = worker.func;
	}
	ThreadWorker& operator = (const ThreadWorker& worker) {
		if (this != &worker)
		{
			thiz = worker.thiz;
			func = worker.func;
		}
		return *this;
	}
	int operator()() {
		if (IsValid())
		{
			return (thiz->*func)();
		}
		return -1;
	}
	bool IsValid() const {
		return (thiz != NULL) && (func != NULL);
	}
	~ThreadWorker() {};

private:
	ThreadFuncBase* thiz;
	FUNCTYPE func;
};



class CThread
{
public:
	bool Start() {
		m_bStatus = true;
		m_hThread = (HANDLE)_beginthread(&CThread::ThreadEntry, 0, this);
		if (!IsVaild()) {
			m_bStatus = false;
		}
		return m_bStatus;
	}

	bool IsVaild() {
		if (m_hThread == NULL || m_hThread == INVALID_HANDLE_VALUE) return false;
		return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
	}

	bool Stop() {
		if (!m_bStatus) return true;
		m_bStatus = false;
		DWORD threadID = GetThreadId(m_hThread);
		bool ret = (WaitForSingleObject(m_hThread, INFINITE) == WAIT_OBJECT_0);
		UpdateWorker();
		return ret;
	}

	void UpdateWorker(const ::ThreadWorker& worker = ::ThreadWorker()) {
		if (m_worker.load() != NULL && m_worker.load()!= &worker)
		{
			::ThreadWorker* pWorker = m_worker.load();
			m_worker.store(NULL);
			delete pWorker;
		}
		
		if (!worker.IsValid())
		{
			m_worker.store(NULL);
			return;
		}
		
		m_worker.store(new ::ThreadWorker(worker));
	}

	// true 表示空闲 false表示已分配
	bool IsIdle() {
		if (m_worker.load() == NULL)
		{
			return true;
		}
		return !m_worker.load()->IsValid();
	}

public:
	CThread() {
		m_hThread = INVALID_HANDLE_VALUE;
		m_bStatus = false;
	}
	~CThread() {
		Stop();
	}

private:
	void ThreadWorker() {
		while (m_bStatus)
		{
			if (m_worker.load()==NULL)
			{
				Sleep(1);
				continue;
			}
			::ThreadWorker worker = *m_worker.load();
			if (worker.IsValid())
			{
				int ret = worker();
				if (ret != 0)
				{
					LOGE(" > Thread error = %d < ", ret);
				}
				if (ret < 0)
				{
					LOGE(" > Thread quit = %d < ", ret);
					m_worker.store(NULL);
				}
			}
			else
			{
				Sleep(1);
			}
		}
		DWORD threadID = GetThreadId(m_hThread);
		LOGI(" Thread quit %d ", threadID);
	}

	static void ThreadEntry(void* arg) {
		CThread* thiz = (CThread*)arg;
		if (thiz)
		{
			thiz->ThreadWorker();
		}
		_endthread();
	}
private:
	HANDLE m_hThread;
	bool m_bStatus; // false 表示线程关闭
	std::atomic<::ThreadWorker*> m_worker;
};

class ThreadPool
{
public:
	ThreadPool(size_t size) {
		m_threads.resize(size);
		for (size_t i = 0; i < size; i++)
		{
			m_threads[i] = new CThread();
		}
	}
	ThreadPool() {

	}
	~ThreadPool() {
		Stop();
		for (size_t i = 0; i < m_threads.size(); i++)
		{
			CThread* pThread = m_threads[i];
			m_threads[i] = NULL;
			delete pThread;
		}
		m_threads.clear();
	}
public:
	bool Invoke() {
		bool ret = true;
		for (auto& x : m_threads)
		{
			if (x->Start() == false) {
				ret = false;
				break;
			}
		}
		if (!ret)
		{
			for (size_t i = 0; i < m_threads.size(); i++) {
				m_threads[i]->Stop();
			}
		}
		return ret;
	}
	void Stop() {
		for (size_t i = 0; i < m_threads.size(); i++) {
			m_threads[i]->Stop();
		}
	}

	// -1 表示失败，线程在忙 大于等于0，表示第n个线程分配来做
	int DispatchWorker(const ThreadWorker& worker) {
		m_lock.lock();
		int index = -1;
		for (int i = 0; i < m_threads.size(); i++)
		{
			if (m_threads[i]->IsIdle()) {
				m_threads[i]->UpdateWorker(worker);
				index = i;
				break;
			}
		}
		m_lock.unlock();
		return index;
	}

	bool CheckThreadVaild(size_t index) {
		if (index < m_threads.size())
		{
			return m_threads[index]->IsVaild();
		}
		return false;
	}
private:
	std::mutex m_lock;
	std::vector<CThread*> m_threads;
};

