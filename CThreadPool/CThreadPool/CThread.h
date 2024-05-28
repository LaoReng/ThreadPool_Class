#pragma once
#include <iostream>
#include "CFunction.h"
// #include <Windows.h> // windows
#include <atomic>
// #include <process.h>  // 线程函数存放位置 // windows

#ifndef WIN32
#include <unistd.h>
#include <signal.h>
#include <pthread.h>  // 线程头文件
#endif // !WIN32



#ifndef TRACE
#define TRACE(str,...) {\
	char output[251]="";\
	snprintf(output, sizeof(output), "%s(%d)" str, __FILE__,__LINE__,##__VA_ARGS__);\
	OutputDebugStringA(output);}
#endif // !TRACE

#ifndef INVALID_HANDLE_VALUE
#ifndef WIN32
#define INVALID_HANDLE_VALUE 0
#endif // !WIN32
#endif // !INVALID_HANDLE_VALUE


/**
* 线程模板类
*/

// 要投递给线程的任务
class ThreadWorker
{
public:
	ThreadWorker()
		: m_function(NULL)
	{}
	/// <summary>
	/// 有参构造函数
	/// </summary>
	/// <typeparam name="_FUNCTION_">任务函数的类型</typeparam>
	/// <typeparam name="..._ARGS_">任务函数的参数</typeparam>
	/// <param name="func">任务函数指针</param>
	/// <param name="...args">任务函数的参数， 这里使用了可变参数模板</param>
	template<class _FUNCTION_, class... _ARGS_>
	ThreadWorker(_FUNCTION_ func, _ARGS_... args)
		: m_function(new CFunction<_FUNCTION_, _ARGS_...>(func, args...))
	{}
	/// <summary>
	/// 析构函数
	/// </summary>
	~ThreadWorker() {}
	/// <summary>
	/// 拷贝构造函数
	/// </summary>
	/// <param name="worker">类的实例化对象</param>
	ThreadWorker(const ThreadWorker& worker) {
		m_function = worker.m_function;
	}
	/// <summary>
	/// 等于号重载
	/// </summary>
	/// <param name="worker">类的实例化对象</param>
	/// <returns>返回值类型为ThreadWorker的引用</returns>
	ThreadWorker& operator=(const ThreadWorker& worker) {
		if (this != &worker) {
			m_function = worker.m_function;
		}
		return *this;
	}
	/// <summary>
	/// 重载括号运算符，仿函数所用
	/// </summary>
	/// <returns>返回值为任务函数的返回值</returns>
	int operator()() {
		if (m_function != NULL) {
			return (*m_function)();
		}
		return -1;
	}
	/// <summary>
	/// 任务是否有效
	/// </summary>
	/// <returns>返回值表示是否有效 true表示有效 false表示无效</returns>
	bool IsValid() const {
		return m_function != NULL;
	}
private:
	// 线程任务函数
	CFunctionBase* m_function;
};


class CThread
{
public:
	// 线程不允许复制构造和等于号重载
	CThread(const CThread&) = delete;
	CThread& operator=(const CThread&) = delete;
public:
	/// <summary>
	/// 无参构造
	/// </summary>
	CThread();
	/// <summary>
	/// 析构函数
	/// </summary>
	~CThread() {
		Stop();
	}
	/// <summary>
	/// 启动线程
	/// </summary>
	/// <returns>返回值表示线程启动是否成功 true表示成功 false表示失败</returns>
	bool Start();
	/// <summary>
	/// 停止线程
	/// </summary>
	/// <returns></returns>
	bool Stop();
	/// <summary>
	/// 是否有效
	/// </summary>
	/// <returns>返回值表示是否有效 true表示有效 false表示无效</returns>
	bool IsValid();
	/// <summary>
	/// 更新线程任务
	/// </summary>
	/// <param name="worker"></param>
	void UpdateWorker(const ThreadWorker& worker = ThreadWorker());
	/// <summary>
	/// 当前线程是否处于空闲状态
	/// </summary>
	/// <returns>返回值表示是否空闲 true表示空闲 false表示已经分配了任务</returns>
	bool IsIdle();
private:
	/// <summary>
	/// 线程的入口函数
	/// </summary>
	/// <param name="arg">传给线程的参数</param>
#ifdef WIN32
	static void ThreadEntry(void* arg) {
		CThread* thiz = (CThread*)arg;
		if (thiz) {
			thiz->ThreadMain();
		}
		_endthread();
	}
#else
	static void* ThreadEntry(void* arg) {
		CThread* thiz = (CThread*)arg;
		if (thiz == NULL)return NULL;
		struct sigaction act = { 0 };
		sigemptyset(&act.sa_mask);
		act.sa_flags = SA_SIGINFO;
		act.sa_sigaction = &CThread::Sigaction;
		sigaction(SIGUSR1, &act, NULL); // 信号注册
		sigaction(SIGUSR2, &act, NULL);
		thiz->ThreadMain();
		// 线程结束处理
		pthread_t thread = pthread_self();
		pthread_detach(thread);
		pthread_exit(NULL);
		return NULL;
	}
	/// <summary>
	/// 处理线程信号
	/// </summary>
	/// <param name="signo">信号编号，区分不同的触发信号</param>
	/// <param name="info"></param>
	/// <param name="context"></param>
	static void Sigaction(int signo, siginfo_t* info, void* context) {
		if (signo == SIGUSR1) {
			// 线程退出信号
			pthread_exit(NULL);
		}
	}
#endif // WIN32
	/// <summary>
	/// 线程主函数
	/// </summary>
	void ThreadMain();
public:
#ifdef WIN32
	HANDLE                     m_hThread;  // 线程句柄
#else
	pthread_t                  m_thread;   // 线程id
#endif // WIN32
	int                        m_iStatus;  // 线程状态, 0表示线程将要关闭  1表示线程正在运行  2表示线程处于暂停状态
	std::atomic<ThreadWorker*> m_worker;   // 线程的任务
};

