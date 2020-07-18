#include "../lib/thread_pool.cpp" 
class thp_test
{
    private:
        std::string s;
    public:
        thp_test():s("hello thread "){};
        void run()
        {
            std::cout<<s<<pthread_self()<<std::endl;
        }
};
int main()
{
    thp::Thread_Pool<thp_test>tp(10);
    sleep(1);
    for(int i=0;i<10;i++)
    {
        tp.append(new thp_test());
        sleep(1);
    }
    tp.stop();
    return 0;
}
