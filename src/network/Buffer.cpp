#include <network/Buffer.h>

#include <algorithm>

#include <unistd.h>
#include <errno.h>

using namespace network;

namespace network
{
	const size_t BUF_INIT_CAPACITY = 1024;
}

/*
char* ByteBuffer::findLastBytes(const std::string& str)
{
	return const_cast<char *>(static_cast<const ByteBuffer&>(*this).findLastBytes(str));
}
*/

const char* ByteBuffer::findLastBytes(const std::string& str, const char* end) const
{
	CHECK(str.size() > 0 && end >= addrOfRead() && end < addrOfWrite());
	std::string::size_type pos = toString().rfind(str, end - addrOfRead());
	if (pos != std::string::npos)
	{
		return addrOfRead() + pos;
	}
	return nullptr;
}

const char* ByteBuffer::findLastBytes(const std::string& str) const
{
	return findLastBytes(str, addrOfRead() + posOfWrite - posOfRead);
}


/*
char* ByteBuffer::findBytes(const std::string& str)
{
	return const_cast<char *>(static_cast<const ByteBuffer&>(*this).findBytes(str));
}
*/

const char* ByteBuffer::findBytes(const std::string& str, const char* start) const
{
	CHECK(str.size() > 0 && start >= addrOfRead() && start < addrOfWrite());
	std::string::size_type pos = toString().find(str, start - addrOfRead());
	if (pos != std::string::npos)
	{
		return addrOfRead() + pos;
	}
	return nullptr;
}

const char* ByteBuffer::findBytes(const std::string& str) const
{
	return findBytes(str, addrOfRead());
}

int ByteBuffer::readFd(int fd)
{
	CHECK(fd >= 0);
	while(1)
	{
		ssize_t num = ::read(fd, addrOfWrite(), writableBytes());
		if (num == 0)
			return static_cast<int>(readableBytes());
		else if (num > 0)
		{
			movePosOfWrite(num);
			if (writableBytes() == 0)
			{
				capacity *= 2;
				buf.resize(capacity);
			}
		}
		else
		{
			LOGERROR(errno);
			return -1;
		}
	}
}

int ByteBuffer::writeFd(int fd)
{
	CHECK(fd >= 0);
	size_t writtenBytes = readableBytes();
	while(readableBytes() > 0)
	{
		ssize_t num = ::write(fd, addrOfRead(), readableBytes());
		if (num < 0)
		{
			LOGERROR(errno);
			return -1;
		}
		movePosOfRead(static_cast<size_t>(num));
	}
	reset();
	return static_cast<int>(writtenBytes);
}

size_t ByteBuffer::readableBytes() const
{
    return (posOfWrite - posOfRead);
}

size_t ByteBuffer::writableBytes() const
{
    return (capacity - posOfWrite);
}

std::string ByteBuffer::toString() const
{
	return std::string(addrOfRead(), readableBytes());
}

void ByteBuffer::reset()
{
	posOfRead = posOfWrite = 0;
}

std::vector<char> ByteBuffer::readBytes(size_t n)
{
	size_t remaining = readableBytes();
	size_t readNum = (n <= remaining ? n : remaining);
    CHECKX((readNum <= remaining), "no enough byte to be read");

    std::vector<char> ret(buf.begin() + posOfRead, buf.begin() + readNum);
    movePosOfRead(readNum);
    return ret;
}

void ByteBuffer::appendBytes(const char* start, size_t size)
{
    if (size > writableBytes())
        spareSpace(size);
    std::copy(start, start + size, addrOfWrite());
    posOfWrite += size;
}

void ByteBuffer::appendBytes(const ByteBuffer& bufIn)
{
	appendBytes(bufIn.addrOfRead(), bufIn.readableBytes());
}

void ByteBuffer::appendBytes(const std::string& strIn)
{
    appendBytes(strIn.c_str(), strIn.size());
}

void ByteBuffer::appendBytes(const std::vector<char> &vec)
{
    appendBytes(&(*vec.cbegin()), vec.size());
}

void ByteBuffer::spareSpace(size_t size)
{
    if (size <= (capacity - posOfWrite + posOfRead))
    {
        std::copy(buf.begin() + posOfRead, buf.begin() + posOfWrite, buf.begin());
        posOfWrite = posOfWrite - posOfRead;
        posOfRead = 0;
    }
    else
    {
        buf.resize(posOfWrite + size);
        capacity = posOfWrite + size;
    }
}

void ByteBuffer::movePosOfWrite(size_t size)
{
    CHECK(size <= writableBytes());
    posOfWrite += size;
}

void ByteBuffer::movePosOfRead(size_t size)
{
	if(size > readableBytes())
	{
    	CHECK(size <= readableBytes());
		size = readableBytes();
	}
    posOfRead += size;
	if (posOfRead == posOfWrite)
		posOfRead = posOfWrite = 0;
}

