#pragma once
#include <conio.h>
#include <mutex>
#include "pch.h"
#include <list>
#include "Log.h"
#include "Thread.h"


template<class T>
class CQueue
{
// 线程安全的队列 IOCP

public:
	enum
	{
		EQNone,
		EQPush,
		EQPop,
		EQSize,
		EQClear
	};

	typedef struct IOCP_Param
	{
		size_t nOperator;
		T strData;
		HANDLE hEvent; //pop
		IOCP_Param(int op, const T& sData, HANDLE event = NULL) {
			nOperator = op;
			strData = sData;
			hEvent = event;

		}
		IOCP_Param() {
			nOperator = EQNone;
		}
	}PARAM, *PPARAM; // 用于投递信息结构体


public:
	CQueue() {
		m_Lock = false;
		m_hCompeletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
		m_hThread = INVALID_HANDLE_VALUE;
		if (m_hCompeletionPort != NULL)
		{
			m_hThread = (HANDLE)_beginthread(&CQueue<T>::threadEntry, 0, this);
		}
	}
	virtual ~CQueue() {
		if (m_Lock) return;
		m_Lock = true;
		PostQueuedCompletionStatus(m_hCompeletionPort, 0, NULL, NULL);
		WaitForSingleObject(m_hThread, INFINITE);
		if (m_hCompeletionPort != NULL)
		{
			HANDLE hTemp = m_hCompeletionPort;
			m_hCompeletionPort = NULL;
			CloseHandle(hTemp);
		}
	}

	bool Pushback(const T& data) {
		PPARAM pParam = new PARAM(EQPush, data);
		if (m_Lock) {
			delete pParam;
			return false;
		}
		bool ret =  PostQueuedCompletionStatus(
			m_hCompeletionPort,
			sizeof(PARAM),
			(ULONG_PTR)pParam,
			NULL
		);
		if (!ret) delete pParam;
		return ret;
	}
	virtual bool Popfront(T& data) {
		HANDLE temphEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		PARAM Param(EQPop, data, temphEvent);
		if (m_Lock) {
			if (temphEvent)
			{
				CloseHandle(temphEvent);
			}
			return false;
		}
		bool ret = PostQueuedCompletionStatus(
			m_hCompeletionPort,
			sizeof(PARAM),
			(ULONG_PTR)&Param,
			NULL
		);
		if (!ret) {
			CloseHandle(temphEvent);
			return false;
		}
		ret = (WaitForSingleObject(temphEvent, INFINITE) == WAIT_OBJECT_0);
		if (ret)
		{
			data = Param.strData;
		}
		return ret;
	}
	size_t Size() {
		HANDLE temphEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		PARAM Param(EQSize, T(), temphEvent);
		if (m_Lock) {
			if (temphEvent)
			{
				CloseHandle(temphEvent);
			}
			return -1;
		}
		bool ret = PostQueuedCompletionStatus(
			m_hCompeletionPort,
			sizeof(PARAM),
			(ULONG_PTR)&Param,
			NULL
		);
		if (!ret) {
			CloseHandle(temphEvent);
			return -1;
		}
		ret = (WaitForSingleObject(temphEvent, INFINITE) == WAIT_OBJECT_0);
		if (ret)
		{
			return Param.nOperator;
		}
		return -1;
	}
	bool Clear() {
		PPARAM pParam = new PARAM(EQClear, T());
		if (m_Lock) {
			delete pParam;
			return false;
		}
		bool ret = PostQueuedCompletionStatus(
			m_hCompeletionPort,
			sizeof(PARAM),
			(ULONG_PTR)pParam,
			NULL
		);
		if (!ret) delete pParam;
		return ret;
	}

	static void threadEntry(void* arg) {
		CQueue<T>* thiz = (CQueue<T>*)arg;
		thiz->threadMain();
		_endthread();
	}

protected:

	virtual void DealParam(PPARAM pParam) {
		switch (pParam->nOperator)
		{
		case EQPush:
		{
			m_lstData.push_back(pParam->strData);
			delete pParam;
			break;
		}
		case EQPop:
		{
			if (m_lstData.size() > 0)
			{
				// 讲道理，只是取 strData
				pParam->strData = m_lstData.front();
				m_lstData.pop_front();
			}
			if (pParam->hEvent != NULL) SetEvent(pParam->hEvent);
			break;
		}
		case EQClear:
		{
			m_lstData.clear();
			delete pParam;
			break;
		}
		case EQSize:
		{
			pParam->nOperator = m_lstData.size();
			if (pParam->hEvent != NULL) SetEvent(pParam->hEvent);
			break;
		}
		default:
		{
			LOGE(" m_lstData ERROR! ");
			break;
		}
		}
	}
	virtual void threadMain() {
		PPARAM pParam = NULL;
		ULONG_PTR CompKey = 0;
		DWORD dwTransfer = 0;
		OVERLAPPED* pOverlap = NULL;
		while (GetQueuedCompletionStatus(m_hCompeletionPort
			, &dwTransfer, &CompKey, &pOverlap, INFINITE))
		{
			if (dwTransfer == 0 || CompKey == NULL)
			{
				printf("thread to close\r\n");
				break;
			}
			pParam = (PPARAM)CompKey;
			DealParam(pParam);
		}
		while (GetQueuedCompletionStatus(m_hCompeletionPort
			, &dwTransfer, &CompKey, &pOverlap, 0))
		{
			if (dwTransfer == 0 || CompKey == NULL)
			{
				printf("thread to close\r\n");
				break;
			}
			pParam = (PPARAM)CompKey;
			DealParam(pParam);
		}
		HANDLE hTemp = m_hCompeletionPort;
		CloseHandle(hTemp);
		m_hCompeletionPort = NULL;
	}

	std::list<T>& GetlstData() {
		return m_lstData;
	}

	bool getAtomicFlag() const { return m_Lock.load(); }
	void setAtomicFlag(bool value) { m_Lock.store(value); }


protected:
	std::list<T> m_lstData;
	HANDLE m_hCompeletionPort;
	HANDLE m_hThread;
    std::atomic<bool> m_Lock; // 队列析构
   // bool m_Lock; // 队列析构
};




template<class T>
class CSendQueue : public CQueue<T> , public ThreadFuncBase
{
public:
	typedef int (ThreadFuncBase::* ECALLBACK)(T& data);
	CSendQueue() {}
	CSendQueue(ThreadFuncBase* obj, ECALLBACK callback) :
		CQueue<T>(), m_base(obj), m_callback(callback)
	{
		m_thread.Start();
		m_thread.UpdateWorker(::ThreadWorker(this, (FUNCTYPE)&CSendQueue<T>::threadTick));
		
	}
	virtual ~CSendQueue() {
		m_base = NULL;
		m_callback = NULL;
	}

protected:
	int threadTick() {
		if (CQueue<T>::GetlstData().size() > 0)
		{
			Popfront();
		}
		Sleep(1);
		return 0;
	}
	virtual bool Popfront(T& data) {
		return false;
	}
	bool Popfront() {
		typename CQueue<T>::IOCP_Param* Param = new typename CQueue<T>::IOCP_Param(CQueue<T>::EQPop, T());
		if (CQueue<T>::getAtomicFlag()) {
			delete Param;
			return false;
		}
		bool ret = PostQueuedCompletionStatus(
			CQueue<T>::m_hCompeletionPort,
			sizeof(*Param),
			(ULONG_PTR)&Param,
			NULL
		);
		if (!ret) {
			delete Param;
			return false;
		}
		return ret;
	}

	virtual void DealParam(typename CQueue<T>::PPARAM pParam) {
		switch (pParam->nOperator)
		{
		case CQueue<T>::EQPush:
		{
			CQueue<T>::GetlstData().push_back(pParam->strData);
			delete pParam;
			break;
		}
		case CQueue<T>::EQPop:
		{
			if (CQueue<T>::GetlstData().size() > 0)
			{
				pParam->strData = CQueue<T>::GetlstData().front();
				if ((m_base->*m_callback)(pParam->strData) == 0) {
					CQueue<T>::GetlstData().pop_front();
				}
			}
			delete pParam;
			break;
		}
		case CQueue<T>::EQClear:
		{
			CQueue<T>::GetlstData().clear();
			delete pParam;
			break;
		}
		case CQueue<T>::EQSize:
		{
			pParam->nOperator = CQueue<T>::GetlstData().size();
			if (pParam->hEvent != NULL) SetEvent(pParam->hEvent);
			break;
		}
		default:
		{
			LOGE(" m_lstData ERROR! ");
			break;
		}
		}
	}



private:
	ThreadFuncBase* m_base;
	ECALLBACK m_callback;
	CThread m_thread;

};

typedef CSendQueue<std::vector<char>>::ECALLBACK SENDCALLBACK;