#include "network/event_channel.h"
#include "network/epoll.h"
#include "libbase/timeutil.h"
#include "libbase/logger.h"

#include <vector>
#include <functional>
#include <iostream>

#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>

ssize_t recv_cnt;
int writes, fired;
int num_pipes, num_actives, num_writes;

std::vector<int> pipes;
std::vector<network::EventChannel*> chans;
network::Epoll* epoll_ptr = NULL;

void ReadCallback(int fd, int idx) {
  char ch;
  recv_cnt += ::recv(fd, &ch, 1, 0);  
  if (writes > 0) {
    int widx = idx + 1;
    if(widx >= num_pipes)
      widx -= num_pipes;
    ::send(pipes[widx*2 + 1], "m", 1, 0);
    --writes;  
    ++fired;
  }
  if (fired == recv_cnt) {
    epoll_ptr->Stop();
  }
}

void WriteCallback() {
}

void CloseCallback() {
}

void RunOnce() {
  for (int i = 0; i < num_pipes; i++) {
    chans[i]->Initialize(std::bind(ReadCallback, chans[i]->Fd(), i), WriteCallback, CloseCallback);
    chans[i]->AddIntoLoopWithRead();
  }
  
  int space = num_pipes / num_actives;
  space *= 2;

  for (int i = 0; i < num_actives; i++) {
    ::send(pipes[i * space + 1], "m", 1, 0);  
  }
  fired = num_actives;
  recv_cnt = 0;
  writes = num_writes;

  int64_t start = libbase::TimeUtil::NowMicros();

  epoll_ptr->Run();

  int64_t span = libbase::TimeUtil::ElapsedMicros(start);
  std::cout << span << " us" << std::endl; 
}

int main(int argc, char* argv[]) {
  num_pipes = 100;
  num_actives = 1000;
  num_writes = 100;
  
  int test_cnt = 25;

  int opt;
  while ((opt = getopt(argc, argv, "n:a:w:t:")) != -1) {
    switch (opt) {
      case 'n' : {
        num_pipes = atoi(optarg);
        break;
      }
      case 'a' : {
        num_actives = atoi(optarg);
        break;
      }
      case 'w' : {
        num_writes = atoi(optarg);
        break;
      }
      case 't' : {
        test_cnt = atoi(optarg);
        break;
      }
      default: {
        std::cerr << "Illegal argument: " << opt << std::endl;
        return 1;
      }
    }      
  }

  struct rlimit rl;
  rl.rlim_cur = rl.rlim_max = num_pipes * 2 + 50;
  if (::setrlimit(RLIMIT_NOFILE, &rl) == -1) {
    std::cerr << "setrlimit error: " << strerror(errno) << std::endl;
    return 1;  
  }

  pipes.resize(2 * num_pipes);
  for (int i = 0; i < num_pipes; ++i) {
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, &pipes[2 * i]) == -1) {
      std::cerr << "socketpair error: " << strerror(errno) << std::endl;
      return 1;  
    }  
  }

  network::Epoll epoll;
  epoll.Initialize();
  epoll_ptr = &epoll;
  
  for (int i = 0; i < num_pipes; i++) {
    chans.push_back(new network::EventChannel(epoll_ptr, pipes[i * 2]));  
  }

  std::cout << "=========start=========\n";
  for (int i = 0; i < test_cnt; ++i) {
    RunOnce();
    std::cout << "writes=" << writes 
    << " fired=" << fired 
    << " received=" << recv_cnt << std::endl;      
  }
  std::cout << "==========end==========\n";

  for (int i = 0; i < num_pipes; ++i) {  
    epoll_ptr->DelChannel(chans[i]);  
    delete chans[i];
  }
  return 0;
}
