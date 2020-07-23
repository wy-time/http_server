#ifndef __HTTP_SERVER_H__
# define __HTTP_SERVER_H__

#include <unistd.h> 
#include <sys/types.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <sys/stat.h> 
#include <cstring> 
#include <algorithm> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <iostream> 
#include <unistd.h> 
#include <mutex> 
#include <condition_variable> 
#include <thread> 
#include <atomic> 
#include <queue> 
#include <memory> 
#include <cassert> 
#include <sys/epoll.h> 
#include <unordered_map> 
#include <sys/signal.h> 
#endif //__HTTP_SERVER_H__
