#ifndef LIBBASE_TIMESTAMP_H
#define LIBBASE_TIMESTAMP_H

#include <libbase/Debug.h>

#include <string>

#include <sys/time.h>
#include <time.h>
#include <string.h>

namespace libbase
{
class TimeStamp
{
public:
	TimeStamp()
	{
		CHECK(gettimeofday(&tv, NULL) == 0);
	} 
	~TimeStamp() {}

	std::string bePretty() const
	{
		char tmBuf[64];
		char buf[64] = {'\0'};

		time_t sec = tv.tv_sec;
		struct tm* secM = localtime(&sec);
		strftime(tmBuf, sizeof tmBuf, "%Y-%m-%d %H:%M:%S", secM);
		snprintf(buf, sizeof buf, "%s.%06ld", tmBuf, tv.tv_usec);
		return std::string(buf, strlen(buf));
	}

	void uptodate()
	{
		CHECK(gettimeofday(&tv, NULL) == 0);
	}

	// add interval seconds
	void addInterval(time_t interval)
	{
		CHECK(interval >= 0);
		tv.tv_sec += interval;
	}

	int64_t diffInMicroSecond(const TimeStamp& rhs) const
	{
		long int diffSec = getSecond() - rhs.getSecond();
		long int diffUsec = getMicroSecond() - rhs.getMicroSecond();
		return (diffSec*1000000 + diffUsec);
	}

	time_t getSecond() const { return tv.tv_sec; } 
	time_t getMicroSecond() const { return tv.tv_usec; }

	bool operator<(const TimeStamp& rhs) const
	{			
		return diffInMicroSecond(rhs) < 0;
	}

	bool operator>(const TimeStamp& rhs) const
	{
		return diffInMicroSecond(rhs) > 0;
	}

	bool operator==(const TimeStamp& rhs) const
	{
		return getSecond() == rhs.getSecond() &&
			   getMicroSecond() == rhs.getMicroSecond();
	}

private:
	struct timeval tv;
};	
}
#endif
