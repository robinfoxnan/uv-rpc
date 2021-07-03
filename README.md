前言：
本项目参考了开源项目 https://github.com/wlgq2/uv-cpp
更改了部分内容，比如缓冲区部分去掉了环形缓冲区和链表，
理由是：环形缓冲区不合适做json和protobuf的编码和解析，我觉得需要一整块的内存；

0) BufferPool 类
当前版本的缓存方法：
a)发送的缓冲区使用的是vector<char>，好处是填充时候经过包装直接给rapidjson作为流使用，
可以保证缓冲区自动根据需要扩展；
b)接收时候，假设最大的包长不超过64K，直接在一个缓冲区中解析；针对每个接收OnRead长度不确定的问题，
如果收到的数据不够解析，则需要缓存到TcpConnection中，临时缓存也使用vector<char>，最大扩展时不会超过最大包长；



1)  Async类的必要性
首先，因为libuv的loop是单线程运行模式，而且只有uv_async_send是线程安全的，
也就是说，所有的read和uv_write之类的操作都应在在loop回调函数中去做，方法是：
建立一个async结构体，初始化结构体，使用uv_async_send发送异步消息，
在异步回调中去写入外部客户需要发送数据；
可以外部建立一个队列，把任务压到队列中，回调时候处理；
Async的封装方法是将回调的std::function压到一个队列中，
uv-cpp的方法是将写操作作为lamda闭包整个打包进去，我不喜欢这样，因为不方便监控队列内容，
我决定将发送队列内置于TcpConnnection，由该类来管理所有的发送的队列；

2）EventLoop类的结构
EventLoop封装了uv_loop，但是需要提供外部接口：runInLoop（），外部客户经过此函数调用发送数据等操作；
因此该类内部集成了Async；

默认的loop只适合做演示，在实际运行中，IO复用需要多个线程，也就是让TCPServer运行在一个线程上loop，
其他新的连接都放到LOOP池中处理，
各个连接收到的任务，经过uv的线程池来处理，处理后经过TcpConnnection发回去；

3)  Timer 类
简单封装了一下；

4）LogWrite类
仅仅封装了几种格式，并没有实现写文件；写文件使用其他的 日志库经过回调来对接实现；


5）TcpConnection  类
客户端可以使用此类直接发起连接，访问服务器

6）ITask类
该类作为rpc传递任务的基类；用户根据自己的需要定义类，添加自己的属性；
ITask需要保证任务在异步计算返回时有唯一的标志符号来识别任务结果，匹配之前的任务；
所以用户需要保证一个唯一编号的string来作为任务的主键；

7）IDispatcher
在TcpConnection 收到对端的数据，uv_read_callback 需要对数据流解码后分解出一个数据包，
并根据数据包的类型二次解码，或者json，或者protobuf ,翻译成Task类，并附带TcpConnection指针传递给上层用户处理；
这里提供了最基本的SimpleDispatcher，该类使用8个字节作为包头来对数据流解包，包长最多支持65536-8的内容；
用户根据自己的需要对解出的内容部分进行业务解码，比如我使用rapidjson来处理，好处前期设计时间短，随时调整比protobuf容易，
即便序列化后可以方便查看；

8）IEncode
在uv_write之前，由客户或者计算单元执行结束后来执行序列化编码；
编码这里需要使用BufferPool 类提供的vector<char>中进行编码，可以保证足够长；
这里约定，不超过64k

9）GlobalConfig
由于不同的用户，或者不同应用使用不同的编码解码工具；
设置好接口以后，并不直接对TcpConnection设置，通过GlobalConfig 全局配置项来决定如何装配：








