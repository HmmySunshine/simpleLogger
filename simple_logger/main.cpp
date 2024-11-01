#include "logger.h"

int main()
{
    

    Logger::EnableFileOutPut();
    std::thread t[10];
    for (int i = 0; i < 10; i++) {
        t[i] = std::thread([=]() {
            LOG_INFO("Hello from thread {}", i);
        });
    }
   
    for (int i = 0; i < 10; i++) {
        if (t[i].joinable()) {
            t[i].join();
        }
    }
    


    return 0;
    
}
