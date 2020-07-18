#ifndef __THREAD_POOL__
#define __THREAD_POOL__
#include "http_server.h" 
namespace thp
{
    template <class T>
    class Thread_Pool
    {
        private:
            int threadnum;//线程池中线程的最大数量
            std::vector<std::thread>_thread;//线程池中的线程
            std::queue<T*>task_queue;//工作队列
            std::mutex _mtx;//互斥锁
            std::condition_variable cv;//条件变量
            std::atomic_bool is_runing;//判断线程池是否在运行中
            void work();//线程工作函数
        public:
            void stop();//关闭线程池
            void init();//初始化线程池
            void append(T*task);//往工作队列中添加任务,参数使用右值应用来减少拷贝
            explicit Thread_Pool(int num):threadnum(num)
            {
                init();
            }
            ~Thread_Pool()
            {
                stop();
            }
            Thread_Pool(const Thread_Pool&)=delete;
            Thread_Pool& operator =(const Thread_Pool& t)=delete;
    };
}
#endif

