#include <network/TcpClient.h>
#include <libbase/Noncopyable.h>

using namespace network;
using namespace libbase;

ByteBuffer ball;

class Client : private Noncopyable
{
public:
	explicit Client(const csocket::SocketAddress& srvAddr)
		: cli(srvAddr), sendBytes(0), recvBytes(0) 
	{}

	void start() { cli.start(); }
	void stop() { cli.stop(); }
	void setEpoll() { cli.setEpoll(); }

private:
	void readCallback(std::shared_ptr<SocketConnection::Connection>& conn,
	ByteBuffer& buf);
	void initCallback(std::shared_ptr<SocketConnection::Connection>& conn);
	TcpClient cli;
	size_t recvBytes;
};

void Client::readCallback(std::shared_ptr<SocketConnection::Connection>& conn,
BytesBuffer& buf)
{
	recvBytes += buf.readableBytes();
	conn.sendData(buf);	
	buf.reset();
}

void Client::initCallback(std::shared_ptr<SocketConnection::Connection>& conn)
{
	conn->sendData(ball);		
}

// just like TcpServer
class ClientCluster : private Noncopyable
{
public:
	ClientCluster(const csocket::SocketAddress& srvAddrIn, int threadSize)
	{
		
	}
	
private:
	csocket::SocketAddress srvAddr;
	std::vector<Client> cliCluster;
	const int threadNum;
	poller::EpollThreadPool threadPool;

};

