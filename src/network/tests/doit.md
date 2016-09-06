- write 如何处理sigpipe的问题(TODO)
若socket的A端口已经执行close（）， 那么当socket另一端的B端口向此socket中发送数据时，××第一次写会收到RST响应，第二次写会收到SIGPIPE信号××

- 取消定时器时需要注意的地方
首先应该明确取消定时器的操作是跨线程调用还是本线程调用，跨线程调用最后还是被转化为单线程执行，所以不用担心;本线程调用只可能是由于在执行timerQueue中的handleRead（）时各个定时器触发程序执行过程中取消了别的定时器, 此时需要记录哪些定时器是被取消了的，定时器重置时，这些取消的定时器则不会重新启动。
- 将sendBuffer中的数据都发送完之后应该关闭事件通知，防止所谓的busy-loop(就是能够向socket发送数据，但是Buffer没有数据发送。)

- 由于采用的是水平触发通知机制，在读到 0 bytes时，是不是需要进行二次验证？

- signal() 和 sigaction（）的区别
signal函数每次设置具体的信号处理函数（非SIG\_IGN）只能生效一次，若需要每次执行相同的信号处理函数，需要在信号处理函数的开始重新设置该信号的信号处理函数，但是从进入信号处理函数到重新设置该信号的信号处理函数中间存在一个时间窗口，在此期间若有另外一个中断信号产生（默认操作是结束进程），则整个进程将会退出。

sigaction（），在信号处理函数被调用时，系统建立的新的信号屏蔽字会包括正在递送的信号，因此在信号处理函数期间，其他的信号会被阻塞到信号处理函数结束为止。另外，响应函数设置后就一直有效，不会被重置。对除SIGALARM以外的所有信号都企图设置SA\_RESTART标志，当系统调用（read, write等）被这些信号中断时会自动重启。不对SIGALARM执行自动重启的标志的原因是希望对I/O操作设置时间限制。

当希望使用相同方式处理信号时，使用sigaction来设置信号函数。当信号只出现并处理一次，可以使用signal。

调用方法：
> #include <signal.h>
> int sigaction(int signum, const struct sigaction *act, struct sigaction* oldact);
> struct sigaction {
>		void     (*sa_handler)(int);
>		void     (*sa_sigaction)(int, siginfo_t *, void *);
>		sigset_t   sa_mask;
>		int        sa_flags;
>		void     (*sa_restorer)(void);
> };
> e.g. struct sigaction sigAct; 
> 	   sigAct.sa\_handler = SIG\_IGN; 
>      sigaction(SIGPIPE, &sigAct, NULL);

- Error(Connection reset by peer)
A端向B端口发送数据时，B端口正要关闭相应的套接字，就会产生ECONNRST，
若A端口继续向B端口发送数据，则会产生SIGPIPE信号。

- 调试Bug时，可以使用自己定制的LOGTRACE() 实时的查看函数调用顺序。

- 不能将std::function<> 转换为对应的函数指针，但是可以将函数指针转换为std::function<>, 当使用C API需要函数指针（如 pthread\_create()）时，可以使用额外的函数将std::function<>包裹，将此额外的函数（非类中的成员函数）传递给C API。
ref: http://stackoverflow.com/questions/10555464/stdfunction-function-pointer

- priority queue 的大小是否需要限制？why？

- 对一个文件描述符进行write() 操作，在关闭此描述符之前使用read()读取此文件描述符，需要使用lseek()重新更改文件的偏移地址（此时偏移地址应该指向文件末端），否则可能什么都读取不到。

- 对于段错误如何有效的定位？
http://blog.clanzx.net/2014/04/17/segfault.html

- 使用webbench做压力测试？
本机可以fork的进程数只有10000多一点，基本可以满足10K的并发。在测试时，由于系统内存只有6GB，所以用来测试并发的文件选用的较小，起初使用一张高质量的图片进行传输，随即发现会导致内存不够引发段错误。
