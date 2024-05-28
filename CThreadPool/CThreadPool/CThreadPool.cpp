#include "CThreadPool.h"

CThreadPool::CThreadPool(size_t size)
{
	m_threads.resize(size);
	for (size_t i = 0; i < size; i++) {
		m_threads[i] = new CThread();
	}
}

CThreadPool::~CThreadPool()
{
	Stop();
	for (size_t i = 0; i < m_threads.size(); i++) {
		delete m_threads[i];
		m_threads[i] = NULL;
	}
	m_threads.clear();
}

bool CThreadPool::Invoke()
{
	bool ret = true;
	for (size_t i = 0; i < m_threads.size(); i++) {
		if (!m_threads[i]->Start()) { // 线程启动失败
			ret = false;
			break;
		}
	}
	if (ret == false) {
		for (size_t i = 0; i < m_threads.size(); i++) {
			m_threads[i]->Stop();
		}
	}
	return ret;
}

void CThreadPool::Stop()
{
	for (size_t i = 0; i < m_threads.size(); i++) {
		m_threads[i]->Stop();
	}
}

int CThreadPool::DispatchWorker(const ThreadWorker& worker)
{
	int index = -1;
	m_lock.lock();
	for (size_t i = 0; i < m_threads.size(); i++) {
		if (m_threads[i]->IsIdle()) {
			m_threads[i]->UpdateWorker(worker);
			index = int(i);
			break;
		}
	}
	m_lock.unlock();
	return index;
}

bool CThreadPool::CheckThreadValid(size_t index)
{
	if (index < m_threads.size()) {
		return m_threads[index]->IsValid();
	}
	return false;
}
