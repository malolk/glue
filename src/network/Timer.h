#ifndef NETWORK_TIMER_H
#define NETWORK_TIMER_H

#include <functional>

#include <libbase/Noncopyable.h>
#include <libbase/TimeStamp.h>
#include <libbase/Debug.h>

namespace network
{
namespace timer
{

typedef std::function<void()> CallbackOnTimeout;
class Timer: private libbase::Noncopyable
{
public:
	Timer(const CallbackOnTimeout& cb, libbase::TimeStamp time, time_t intervalIn = 0): 
		expiration(time), 
		timeoutCb(cb), 
		interval(intervalIn)
	{
		repeated = ( interval <= 0 ? false : true);
	}

	~Timer() {}

	void timeout() 
	{ 
		CHECK(timeoutCb);
		timeoutCb(); 
	}

	void restart()
	{
		if (repeated)
		{
			expiration.uptodate();
			expiration.addInterval(interval);
		}
		else
		{
			LOGWARN("this timer is unable to repeat");
		}
	}
	
	libbase::TimeStamp getExpiration() const
	{
		return expiration;
	}

	bool isRepeated() { return repeated; }
private:
	libbase::TimeStamp expiration;
	CallbackOnTimeout timeoutCb;
	time_t interval;
	bool repeated;
};

}	
}
#endif
