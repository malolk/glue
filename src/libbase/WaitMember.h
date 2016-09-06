#ifndef LIBBASE_WAITMEMBER_H
#define LIBBASE_WAITMEMBER_H

#include <libbase/Cond.h>
#include <libbase/MutexLock.h>
#include <libbase/Noncopyable.h>
#include <libbase/Debug.h>

#include <iostream>

namespace libbase
{
class WaitMember: private Noncopyable
{
public:
	explicit WaitMember(int num):
	memNum(num), mu(), cond(mu)
	{ }

	void done()
	{
		MutexLockGuard m(mu);
		CHECK(memNum > 0);
		--memNum;
		while(memNum != 0)
		{
			cond.wait();
		}
		cond.notifyAll();	
		std::cout << "WAITOVER\n";
	}
private:
	int	memNum;		
	mutable MutexLock mu;
	Cond cond;
};

}

#endif  //LIBBASE_WAITMEMBER_H
