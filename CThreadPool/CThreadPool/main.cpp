#include <cstdio>
#include "CThreadPool.h"


typedef int SOCKET;

class TestThreadPool
{
public:
	TestThreadPool()
		: m_ThreadPool(5)
	{
		bool ret = m_ThreadPool.Invoke();
		if (ret == false) {
			printf("线程池启动失败\n");
		}
	}

	static int MyWorker(const char* buf, SOCKET client) {
		printf("任务函数被执行\r\n");
		return 0;
	}

	~TestThreadPool() {
		m_ThreadPool.Stop();
	}

	int Dispose()
	{
		printf("执行到这里\n");
		char buf[10] = "1111";
		SOCKET clientSocket = 10;
		ThreadWorker worker(&TestThreadPool::MyWorker, buf, clientSocket);
		m_ThreadPool.DispatchWorker(worker);
		return 0;
	}

private:
	CThreadPool   m_ThreadPool;
};

int main()
{
	TestThreadPool th;
	th.Dispose();
	printf("按任意键输出结果:\n");
	getchar();
}