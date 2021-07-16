# 前言

本项目主要来源是为了分布式CPU密集型计算提供一个RPC框架；

因为在某些环境下不能联网，需要依赖的库越少越好，而且需要跨平台，所以百度rpc被排除了。

谷歌的rpc在下载过程中遇到很多问题，而且有人提及代码很多bug，还是不考虑了。

本项目参考了开源项目 https://github.com/wlgq2/uv-cpp
更改了部分内容，比如缓冲区部分去掉了环形缓冲区和链表，
理由是：环形缓冲区不合适做json和protobuf的编码和解析，我觉得需要一整块的内存；

这里简单介绍一下整个项目的结构以及在使用中需要了解的东西。

<img src=".\doc\overview.png" alt="overview" style="zoom:80%;" />

概述：客户端和服务器端都使用GlobalConfig作为全局配置来对接各个子模块：编码、解码分发；

客户端执行流程：

1. 客户端发送数据，sendMsg会写入到发送队列；放入发送队列过程前就调用encoder执行编码，将发送事件关联发送缓冲区与任务，压入队列；
2. 然后使用异步事件通知uv_loop在回调中执行真正的发送；
3. 收到消息时，SimpleMsgDispatcher先进行数据分包，之后调用全局指定的解码器进行解码处理；用户在实现此解码器的时候可以设置用户回调函数通知用户，用于匹配异步调用结果。

服务端执行流程：

1. 服务端启动时候，会调用连接管理器ConnectionManager创建合适个数的EventLoop，并放置于对应个数的线程中执行循环；

2. 为了提速工作线程池Workpool，提前创建好各种工作线程，需要执行一个任务，所以ConnectionManager在创建线程后，会使用第一个EventLoop上使用lamda语句执行一个简单任务，任务执行时uv库会生成工作线程池，回调输出“worker threads init ok...”。

   工作线程池的线程个数是通过环境变量设置的，具体在GlobalConfig::init()中设置，默认是64个。

   至此，为客户端准备的loop线程池以及工作线程池都准备好了；

3. 服务器TcpServer启动内置的loop循环，开始listen端口，等待连接；

4. 当有新的连接的时候，直接调用ConnectionManager::onNewConnection(server, status)，函数内部自动创建新的连接实例，并分配一个EventLoop给此连接使用；也就是说多个连接共用IO_LOOPS个IO线程，默认IO_LOOPS设置为4个；具体需要根据环境来调整，作为CPU密集型的服务，其实只有一个客户端，所以就不用建立这么IO线程，有一个就够了；或者在TcpServer创建时候通过构造函数执行IO线程个数；

5. 单个的TcpConnection收到数据的时候，会调用解码器解码；解码后有2种方式处理任务，比如处理的任务很短（微秒级），可以在直接执行任务并返回数据；或者（计算时间较长，在毫秒级别）可以在解码后启动一个工作线程任务（参考  PingDispatch2Worker.h）, 调用WorkerPool::addToWorkQueue(task)安排一个任务，工作线程中根据任务类型选择合适的IWork执行需要的工作任务，并把任务结果保存到Task实例中，当工作任务结束的回调执行时，线程处于TcpConnection所在的loop中，适合发送结果数据；

6. 发送结果数据前会执行编码，与客户端发送数据流程相同；

示例：samples\pingTest 目录下提供了最基本的发送和应答的客户端和服务端，这里的编码直接使用了发送流水号的字符串。



# 1)  BufferPool 类

当前版本的缓存方法：
a)发送的缓冲区最开始使用的是vector<char>，好处是填充时候经过包装直接给rapidjson作为流使用，
可以保证缓冲区自动根据需要扩展；但是测试后发现vector的追加效率太低了，于是自己封装了一个 CharVector，
这个类在TcpConnection中也有使用；
b)接收时候，假设最大的包长不超过64K，直接在一个缓冲区中解析；针对每个接收OnRead长度不确定的问题，
如果收到的数据不够解析，则需要缓存到TcpConnection中，临时缓存也使用CharVector，最大扩展时不会超过最大包长；

# 2)  Async类的必要性

首先，因为libuv的loop是单线程运行模式，而且只有uv_async_send是线程安全的，
也就是说，所有的read和uv_write之类的操作都应在在loop回调函数中去做，方法是：
建立一个async结构体，初始化结构体，使用uv_async_send发送异步消息，
在异步回调中去写入外部客户需要发送数据；
可以外部建立一个队列，把任务压到队列中，回调时候处理；
Async的封装方法是将回调的std::function压到一个队列中，
uv-cpp的方法是将写操作作为lamda闭包整个打包进去，我不喜欢这样，因为不方便监控队列内容，
我决定将发送队列内置于TcpConnnection，由该类来管理所有的发送的队列；

uv-cpp的方法也有一个好处，就是少了一次同步访问队列的性能损失；不过std::mutex性能还算可以。

# 3) EventLoop类的结构

EventLoop封装了uv_loop，但是需要提供外部接口：runInLoop（），外部客户经过此函数调用发送数据等操作；
因此该类内部集成了Async；

默认的loop只适合做演示，在实际运行中，IO复用需要多个线程，也就是让TCPServer运行在一个线程上loop，
其他新的连接都放到LOOP池中处理，
各个连接收到的任务，经过uv的线程池WorkerPool来处理，处理后经过TcpConnnection发回去；

# 4)  UvTimer 类

简单封装了一下；并测试了一下，在windows10和ubuntu18.04下都可以实现1ms粒度的定时器，精度会有0.5ms的偏差；



# 5)  LogWriter类和log4wraper类

两个类使用了共同的一个类引用，使用的时候可以简单的包含头文件CommonLog.h，通过宏选择使用哪个类；

LogWriter仅仅封装了几种格式，并没有实现写文件；写文件使用其他的 日志库经过回调来对接实现；

log4wrapper封装了log4cpp；这个库似乎不再更新了，我猜性能已经达到了极致；

自己测试一下性能：

1）window10 + vs2017: 4个线程，每个线程10000条数据，debug共1.7秒 release 0.46秒

2）window10 + vs2017:1线程40000条，debug是1.6秒；release 0.375秒

3）ubuntu18.04 虚拟机: 4个线程，每个线程10000条数据，共0.4秒

3）ubuntu18.04 虚拟机: 1个线程，每个线程40000条数据，共0.145秒

# 6)  TcpConnection  类

客户端可以使用此类直接发起连接，访问服务器；

服务器端每次新接受一个连接，都生成一个该实例；



# 7)  TcpServer类和ConnectionManager类

TcpServer类封装了一个TCP服务；ConnectionManager配合服务类管理各种客户端的连接；



# 8)  IDispatcher类与SimpleMsgDispatcher类

目前使用的代码格式如下图：

<img src=".\doc\packet_ch.png" alt="packet_ch" style="zoom:80%;" />

8字节的头部数据是固定的；保留字可以作为标记识别数据包是否合法，2字节 的数据长度可以最多支持64K的内容；其实一般我们并不需要这么大的数据；在收到对端的数据，uv_read_callback 需要对数据流解码后分解出一个数据包，
并根据数据包的类型二次解码，或者json，或者protobuf ,翻译成Task类，并附带TcpConnection指针传递给上层用户处理；

IDispatcher主要是用于收到数据后，使用合适的方式处理数据包，此类仅仅是一个接口；SimpleMsgDispatcher实现了对数据流的分割；主要原理是在TcpConnection中提供一个CharVector缓冲区，用来保存没有处理的数据，此数据不会超过一个完整数据包（否则就被处理了）；当新来数据时，会遇到6种情况：（SimpleMsgDispatcher::onMessage）

1) 剩余 >= 头长，同时 >= 完整包长：不存在，错误；（这里其实也做了代码处理）
2) 剩余 >= 头长，同时 < 完整包，新来可以补足一个完整包：拷贝补足，解析并清空，处理剩余的
3) 剩余 >= 头长，同时 < 完整包，新来还不够一个完整包：直接拷贝，不处理了

4) 剩余 < 头长， 同时 新数据也不能补足一个头长：直接拷贝不处理
5) 剩余 < 头长,  新数据可以补足一个头，但是不够一个包长：直接拷贝不处理

6) 剩余 < 头长， 新数据可以补足一个头，也够一个包长：拷贝补足一个包长,解析并清空，处理剩下新来的

SimpleMsgDispatcher每次分割出一个数据包之后，会调用虚函数 onMessageParse(DATA_HEADER * header, char *buf, unsigned long len, TcpConnection * conn)进行处理，此函数中用户根据自己的需要对解出的内容部分进行业务解码；可以新建一个子类，并重写此函数，比如在onMessageParse（）中调用rapidjson来处理，或者使用protobuf解码；所以此函数提供了数据包头结构体，以及数据缓冲的头以及数据长度，TcpConnection参数的主要作用是需要关联到Task类，在任务线程处理 后，使用该连接发送会对端，所以此参数不可少。

备注：在uv_read_callback相关的代码中，内存由BufferPool类自动管理，所以在回调函数中，客户解码后 生成Task类实例，不需要关心内存释放，SimpleMsgDispatcher在处理每次新来的数据后，自动归还内存池。

# 9)  ITask类

该类作为rpc传递任务的基类；用户根据自己的需要定义类，添加自己的属性；
ITask需要保证任务在异步计算返回时有唯一的标志符号来识别任务结果，匹配之前的任务；
所以用户需要自己保证一个唯一编号，或者使用string或者使用int64来作为任务的主键；具体本项目并没有封装此类细节；

需要用户根据自己的需要，在客户端管理异步任务，并且在编解码时候做响应的处理。

# 10)  IEncode类

每次发送数据之前都需要对数据进行编码，所以用户在客户端和服务端分别提供编码的类，供系统调用；

具体来说，是在uv_write之前，由客户或者计算单元执行结束后来执行序列化编码；此
编码这里需要使用BufferPool 类提供的vector<char>中进行编码，可以保证足够长；
这里约定，不超过64k。

# 11)  IWork类

如果服务端使用工作线程池来处理耗时的任务，需要在GlobalConfig中注册任务类型以及工作者的对应关系，

```
// 2）set message encoder before send 
	std::shared_ptr<PongEncode> encoder = std::make_shared<PongEncode>();
	GlobalConfig::setEncoder(encoder);
```

在解码器中将任务转发到线程池：

```
virtual void onMessageParse(DATA_HEADER * header, char *buf, unsigned long len, TcpConnection *conn) override
		{
			if (msgCb != nullptr)
				msgCb(buf, len);
			TaskPtr task = std::make_shared<ITask>();
			task->taskIdStr = std::string(buf, len);
			
			task->setConnection(conn);
			WorkerPool::addToWorkQueue(task);
		}
```

线程池的回调函数会根据任务类型寻找合适的工作者。

所以有很多不同的计算任务类型时候，可以每个类型封装为一个Work类型，并注册。

# 12)  GlobalConfig动态状态

由于不同的用户，或者不同应用会使用不同的编码解码工具；
设置好接口以后，并不直接对TcpConnection设置，通过GlobalConfig 全局配置项来决定如何装配：

比如samples/pingTest/client中的示例客户端使用如下代码初始化：

```
void testPing()
{
	GlobalConfig::init();
	// 1）user should rewrite this class，override virtual funciton  'onMessageParse()'；
	std::shared_ptr<PongDispatcher> dispatcher = std::make_shared<PongDispatcher>();
	GlobalConfig::setMsgDispatcher(dispatcher);

	// 1.1)or set the callback to parse data-part of the packet
	dispatcher->msgCb = [&](char *buf, unsigned long len)
	{
		uint64_t id = _atoi64(buf);
		findTask(id);
	};

	// 2）set message encoder before send 
	std::shared_ptr<PingEncode> encoder = std::make_shared<PingEncode>();
	GlobalConfig::setEncoder(encoder);

	EventLoopPtr loopPtr = std::make_shared<EventLoop>();
	conn = std::make_shared<TcpConnection>(loopPtr);
	conn->setConnectCb([](int status)
	{
		if (status < 0)
		{
			printf("connect error%d\n", status);
		}
		else
		{
			printf("connect ok\n");
		}
	});

	conn->connect("127.0.0.1", 80);
	conn->start();
}
```

samples/pingTest/server中的服务代码如下：

```
// decode in dispatcher, and do work in worker thread
void serverPongEn()
{
	GlobalConfig::init();
	// 1）user should rewrite this class，override virtual funciton  'onMessageParse()'；
	std::shared_ptr<PingDispatch2Worker> dispatcher = std::make_shared<PingDispatch2Worker>();
	GlobalConfig::setMsgDispatcher(dispatcher);

	std::shared_ptr<WorkAddNum> workerPtr = std::make_shared<WorkAddNum>();
	GlobalConfig::addWorkType("IWork", std::dynamic_pointer_cast<IWork>(workerPtr));


	// 2）set message encoder before send 
	std::shared_ptr<PongEncode> encoder = std::make_shared<PongEncode>();
	GlobalConfig::setEncoder(encoder);

	TcpServer server(5);
	server.bindAndListen("0.0.0.0", 80, 128);

	server.start();
}
```

备注：在屏幕输出消息是十分耗时的操作，可以使用log4cpp的配置打开输出选项；但是服务端除了监控必要的数据，尽量不要输出，会严重影响性能。