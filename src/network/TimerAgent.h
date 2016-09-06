#ifndef NETWORK_TIMERAGENT_H
#define NETWORK_TIMERAGENT_H

#include <network/Timer.h>

#include <memory>

namespace network
{
namespace timer
{

/*
class TimerAgent
{
public:
	TimerAgent():
	weakPtr(nullptr), binded(false) {}
	explicit TimerAgent(const std::shared_ptr<Timer>& sharedPtr)
	:weakPtr(sharedPtr), binded(true)
	{}

		
	TimerAgent& operator=(TimerAgent& rhs)
	{
			
	}	
	
	~TimerAgent() {}

	std::shared_ptr<Timer> isLived() const
	{
		return weakPtr.lock();
	}

	void bind(const std::shared_ptr<Timer>& sharedPtr)
	{
		weakPtr = sharedPtr;
		binded = true;	
	}
	
	bool isBinded() { return binded; }
private:
	std::weak_ptr<Timer> weakPtr;
	bool binded;
};
*/

}	
}
#endif
