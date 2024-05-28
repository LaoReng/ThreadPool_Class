#include "CThread.h"

bool CThread::Start()
{
#ifdef WIN32
	m_iStatus = 1;
	m_hThread = (HANDLE)_beginthread(&CThread::ThreadEntry, 0, this);
	if (!IsValid()) {
		m_iStatus = 0;
	}
	return m_iStatus == 1;
#else
	m_iStatus = 1;
	do {
		pthread_attr_t attr;
		int ret = 0;
		ret = pthread_attr_init(&attr);
		if (ret != 0) {
			m_iStatus = 0;
			break;
		}
		ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE); // PTHREAD_CREATE_DETACHED当这个线程结束了（调用这个结束）就完了，不需要主线程再去join它了
		if (ret != 0) {
			m_iStatus = 0;
			break;
		}
		// 创建线程
		ret = pthread_create(&m_thread, &attr, &CThread::ThreadEntry, this);
		if (ret != 0) {
			m_thread = INVALID_HANDLE_VALUE;
			m_iStatus = 0;
			break;
		}
		ret = pthread_attr_destroy(&attr);
		if (ret != 0) {
			m_thread = INVALID_HANDLE_VALUE;
			m_iStatus = 0;
			break;
		}
	} while (false);
	return m_iStatus == 1;
#endif // WIN32
}

bool CThread::Stop()
{
#ifdef WIN32
	if (m_iStatus == 0)return true;
	m_iStatus = 0;
	DWORD ret = WaitForSingleObject(m_hThread, 1000);
	if (ret == WAIT_TIMEOUT) {
		TerminateThread(m_hThread, -1);
	}
	UpdateWorker();
	return ret == WAIT_OBJECT_0;
#else
	if (m_iStatus == 0)return true;
	m_iStatus = 0;
	if (m_thread != INVALID_HANDLE_VALUE) {
		pthread_t thread = m_thread;
		m_thread = INVALID_HANDLE_VALUE;
		timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 100 * 1000000;
		int ret = pthread_timedjoin_np(thread, NULL, &ts);
		if (ret == ETIMEDOUT) {
			// 等待超时
			pthread_detach(thread);
			pthread_kill(thread, SIGUSR1);
		}
	}
	UpdateWorker();
	return true;
#endif // WIN32
}

bool CThread::IsValid()
{
#ifdef WIN32
	if ((m_hThread == NULL) || (m_hThread == INVALID_HANDLE_VALUE))return false;
	return WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT;
#else
	if (m_thread == INVALID_HANDLE_VALUE)return false;
	return ESRCH != pthread_kill(m_thread, 0); // ESRCH 表示线程不存在了
#endif // WIN32
}

void CThread::UpdateWorker(const ThreadWorker& worker)
{
	if ((m_worker.load() != NULL) && (m_worker.load() != &worker/*代表是新的worker不是之前的，就需要把之前的释放掉*/)) {
		::ThreadWorker* pWorker = m_worker.load();
		m_worker.store(NULL);
		delete pWorker;
	}
	if (m_worker.load() == &worker) // 如果投递的任务指针一样，则直接返回
	{
		return;
	}
	if (!worker.IsValid()) {
		m_worker.store(NULL);
		return;
	}
	ThreadWorker* pWorker = new ThreadWorker(worker);
	if (pWorker == NULL) {
		printf("%s(%d)<%ld>:线程结束运行\r\n", __FILE__, __LINE__, m_thread);
		// TRACE("内存分配失败\r\n");
		return;
	}
	m_worker.store(pWorker);
}

bool CThread::IsIdle()
{
	if (m_worker.load() == NULL)return true;
	// 任务不为空，下面判断任务是否是无效
	return false == m_worker.load()->IsValid();
}

void CThread::ThreadMain()
{
	while (m_iStatus == 1) {
		if (m_worker.load() == NULL) {
			// 当前没有任务需要执行
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000000);  // 1S
#endif // WIN32
			continue;
		}
		ThreadWorker worker = *m_worker.load();
		// printf("%s(%d)<%ld>:接收到任务\r\n", __FILE__, __LINE__, m_thread);
		if (worker.IsValid()) {
#ifdef WIN32
			if (WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT) {
#else

			if ((m_thread != INVALID_HANDLE_VALUE) && (ESRCH != pthread_kill(m_thread, 0))) { //ESRCH 3 ESRCH表示线程不存在了
#endif // WIN32
				int ret = worker(); // 执行任务
				printf("%s(%d)<%ld>:ret %d\r\n", __FILE__, __LINE__, m_thread, ret);
				// TODO:任务执行完对返回值的处理
				UpdateWorker();
			}
			else {
				// 线程结束
				printf("%s(%d)<%ld>:线程结束运行\r\n", __FILE__, __LINE__, m_thread);
				break;
			}
		}
		else {
#ifdef WIN32
			Sleep(1);
#else
			usleep(1000000);  // 1S
#endif // WIN32
		}
	}
}

CThread::CThread()
{
#ifdef WIN32
	m_hThread = INVALID_HANDLE_VALUE;
#else
	m_thread = INVALID_HANDLE_VALUE;
#endif // WIN32
	m_iStatus = 0;
}
