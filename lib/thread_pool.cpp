#include "thread_pool.h" 

template<class T>
void thp::Thread_Pool<T>::init()
{
    if(is_runing==false)
    {
        is_runing=true;
        for(int i=0;i<threadnum;i++)
        {
            _thread.emplace_back(std::thread(&thp::Thread_Pool<T>::work,this));//创建线程
        }
    }
}

template<class T>
void thp::Thread_Pool<T>::work()
{
    while(is_runing)
    {
        T* task=nullptr;
        {//函数块，保证uniquelock析构时解锁
            std::unique_lock<std::mutex> lk(_mtx);//给队列加锁
            if(!task_queue.empty())
            {
                task=task_queue.front();
                task_queue.pop();
            }else
                cv.wait(lk,[this]()->bool{return !is_runing||!task_queue.empty();});
        }
        if(task!=nullptr)
            task->run();//调用任务类的run函数
    }
    return ;
}
template<class T>
void thp::Thread_Pool<T>::append(T*task)
{
    if(is_runing)
    {
        {
            std::unique_lock<std::mutex>lk(_mtx);//队列加锁
            task_queue.push(task);
        }
        cv.notify_one();//唤醒一个线程
    }
}

template<class T>
void thp::Thread_Pool<T>::stop()
{
    is_runing=false;
    cv.notify_all();//唤醒所有线程
    for(auto &t:_thread)//等待所有线程退出
    {
        if(t.joinable())
            t.join();
    }
    while(!task_queue.empty())//工作队列中还有
    {
        delete task_queue.front();
        task_queue.pop();
    }
}
