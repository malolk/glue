#ifndef NETWORK_BUFFER_H
#define NETWORK_BUFFER_H

#include <libbase/Debug.h>

#include <vector>
#include <string>

namespace network
{

extern const size_t BUF_INIT_CAPACITY;

// copable, default copy constructor, assignment operator and destructor is ok
class ByteBuffer 
{
public:
    explicit ByteBuffer(size_t capIn = BUF_INIT_CAPACITY):
        capacity(capIn), 
		posOfRead(0), 
		posOfWrite(0)
    {
        buf.resize(capacity, 0);
    }

	//fix move constructor
	
    ~ByteBuffer() {}

    // could make it const?
    size_t readableBytes() const;
    size_t writableBytes() const;
	const char* findBytes(const std::string& ) const;
	const char* findBytes(const std::string&, const char*) const;
//	char* findBytes(const std::string& );
	const char* findLastBytes(const std::string& ) const;
	const char* findLastBytes(const std::string&, const char*) const;
//	char* findLastBytes(const std::string& );
    char* addrOfWrite() { return &*(buf.begin() + posOfWrite); }
    const char* addrOfWrite() const { return &*(buf.begin() + posOfWrite); }
//  char* addrOfRead() { return &*(buf.begin() + posOfRead); }
    const char* addrOfRead() const { return &*(buf.begin() + posOfRead); }
    void movePosOfWrite(size_t size );
    void movePosOfRead(size_t size);
	void reset();
    std::vector<char> readBytes(size_t );
    //char* writeBytesOutside(int );
    void appendBytes(const char *, size_t );
    void appendBytes(const std::vector<char>& );
    void appendBytes(const std::string& );
	void appendBytes(const ByteBuffer& );
	int readFd(int fd);
	int writeFd(int fd);
	std::string toString() const;
	size_t cap() const { return capacity; }
	size_t readPos() const { return posOfRead; }
	size_t writePos() const { return posOfWrite; }
private:
    void spareSpace(size_t );
    std::vector<char> buf;
    size_t capacity;
    size_t posOfRead;
    size_t posOfWrite;
};
}
#endif // NETWORK_BUFFER_H
