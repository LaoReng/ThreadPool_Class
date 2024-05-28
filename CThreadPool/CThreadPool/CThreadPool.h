#pragma once
#include "CThread.h"
#include <mutex>
#include <vector>

class CThreadPool
{
public:
	/// <summary>
	/// 有参构造函数
	/// </summary>
	/// <param name="size">指定初始化的线程数</param>
	CThreadPool(size_t size);
	/// <summary>
	/// 析构函数
	/// </summary>
	~CThreadPool();
	/// <summary>
	/// 启动线程池
	/// </summary>
	/// <returns>返回值表示启动是否成功 true表示成功 false表示失败</returns>
	bool Invoke();
	/// <summary>
	/// 停止线程池运行
	/// </summary>
	void Stop();
	/// <summary>
	/// 向线程池投递任务
	/// </summary>
	/// <param name="worker">要投递的任务</param>
	/// <returns>返回值表示投递是否成功， 
	/// 返回值大于0表示第n个线程分配来执行这个任务，返回值为-1表示投递失败
	/// </returns>
	int DispatchWorker(const ThreadWorker& worker);
	/// <summary>
	/// 查看指定线程是否有效
	/// </summary>
	/// <param name="index">查看指定线程的索引</param>
	/// <returns>返回值表示指定线程是否有效， true表示有效 false表示无效</returns>
	bool CheckThreadValid(size_t index);
private:
	std::mutex            m_lock;    // 用于多线程访问安全
	std::vector<CThread*> m_threads; // 线程的容器
};

