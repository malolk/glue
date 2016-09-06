#include <network/Buffer.h>

#include <iostream>
#include <vector>
#include <string>

using namespace network;

int main()
{
	const char test[] = "asdfghjkl;";
	const char test1[] = "qwertyuiop";
	const char test2[] = "zxcvbnm,./";
	const std::string conStr = "1234567890";

	ByteBuffer buf;
	CHECKX(buf.readableBytes() == 0, "intiate readable size is non-zero");
	CHECKX(buf.writableBytes() == BUF_INIT_CAPACITY, "intiate writable size is not BUF_INIT_CAPACITY");
	
	buf.appendBytes(test, 10);
	std::cout << buf.toString() << std::endl;
	CHECK(buf.readableBytes() == 10);

	std::vector<char> vecOfChar(test1, test1+10);
	buf.appendBytes(vecOfChar);
	std::cout << buf.toString() << std::endl;
	CHECK(buf.readableBytes() == 20);

	std::cout << "-----------------\n";	
	buf.appendBytes(conStr);
	std::cout << buf.toString() << std::endl;
	CHECK(buf.readableBytes() == 30);

	std::cout << "-----------------\n";	
	ByteBuffer buf2;
	buf2.appendBytes(test2, 10);
	std::cout << buf2.toString() << std::endl;

	buf.appendBytes(buf2);
	std::cout << buf.toString() << std::endl;
	CHECK(buf.readableBytes() == 40);
	std::cout << "-----------------\n";	

	std::vector<char> vecOfChar2 = buf.readBytes(10);
	std::cout << std::string(&(*vecOfChar2.begin()), vecOfChar2.size()) << std::endl;
	std::cout << buf.toString() << std::endl;
	CHECK(buf.readableBytes() == 30);

	buf.reset();
	std::cout << buf.toString() << std::endl;
	CHECK(buf.readableBytes() == 0);
	CHECK(buf.addrOfWrite() == buf.addrOfRead());

	std::cout << "PASS ^_^" << std::endl;
}

