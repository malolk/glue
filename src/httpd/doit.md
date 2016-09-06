### httpd 的实现笔记

#### 模块功能描述
- HttpParse： 主要完成的功能是对http报文进行解析，包括判定接收报文的完整性和对报文头的解析功能，若请求信息接收完整则使用HttpRequest和HttpResponse对请求信息进行处理，并发送HttpResponse
- HttpRequest： 根据请求信息判定请求是否有效，若有效则对其进行处理，现阶段先完成对静态文件的Get要求，后续加上CGI的请求处理
- HttpResponse： 根据要发送的信息形成响应报文并发送回去
- HttpServer： 在底层的TcpServer中注入回调函数

#### 特性增加
- [ ]支持keep-alive
- 解除Timer的shared\_ptr封装
- 简化TimerQueue中TimerKey的设计

#### 实现笔记
- 为什么在多线程的环境下使用fork是比较危险的？
Ans： 比如当前进程中有线程A， B， C， D， 此时B线程正在malloc分配内存，由于malloc内置锁，当B线程获得malloc的锁时，A线程正好在使用fork来新建子进程，子进程中只会拷贝A线程到进程地址空间中，即子进程中只会存在A线程的拷贝，由于在线程A中malloc的锁是被其他线程占用的，因此子进程中A线程的拷贝中malloc的锁处于一种未释放的状态，此时子进程中若使用malloc将会一直阻塞。
解决方案：在fork后先关闭掉继承自父进程的描述符（不包括stdin、stdout、stdcerr， 然后使用execl（）来运行新的进程环境，将heap、stack等信息全部换掉，这样子进程的操作也不会受到父进程中多线程的影响。

- 如何支持keep-alive以及pipeline？（参考nginx的实现）
判定持久连接：解析http request的header Connection是否带有Keep-Alive，若有则判定此连接为持久连接（其实只需要在第一次请求中申明Keep-Alive）;若Connection后面的参数为Close，则判定为短时连接; 若无Connection的首部且HTTP的版本为1.1及以上则将此链接判定为持久连接。
如何处理pipelined请求：读取recvBuffer中接收完整的请求，将完整请求根据接收先后次序放入处理队列，这样先接收到的请求会先处理。
定时器的设置：1）初步的处理方式是，使用server中默认的timeout将时间窗口划分为一个个子窗口，判断前一个窗口中是否有新的请求到， 若在此期间存在新的请求，则根据当前是否在处理请求来决定是否重新启动定时器; 若在此期间不存在新的请求，则判断此连接已经超时，开始关闭连接。初始阶段当server端处理完当前连接中所收到的requests（多个request可能会使用pipelined的方式发送过来），向TimerQueue中添加timeout定时器。可以判断每一个时间窗口中只会存在一个定时器，当后续请求在timeout之前到来时不需要取消定时器，只有当前一个定时器已经timeout了才重新启动定时器。在HTTP 规定timeout为连接空闲的时间，此种方式的处理只能保证连接超时2\*timeout时一定会将连接关闭。但是，这种简单的处理使得编程变得更加简单，可以非常容易的保证程序的正确性。2）更简单的timeout处理方式是将timeout指定为整个连接持续的时间。3）严格遵守HTTP规范的超时方案是，在当前窗口中若有后续请求发送过来，就应该将之前设置的定时器取消（具体怎么取消可以使用类似lazy deletion的方法），处理完当前接收到的请求后应该重新向epoller注册定时器。这种处理方式比较严格，在一个时间窗口之内会存在多个定时器，因此必须处理好多个定时器之间的时序。本实现主要实现第三种方案。
持久连接的回应：数据编码方式采用Transfer-encoding（块传输）的方式，最后一个长度为0的块代表传输结束。（具体实现还需参考相应的资料）。
