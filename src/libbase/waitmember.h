#ifndef GLUE_LIBBASE_WAITMEMBER_H_
#define GLUE_LIBBASE_WAITMEMBER_H_

#include "libbase/logger.h"
#include "libbase/condvar.h"
#include "libbase/mutexlock.h"
#include "libbase/noncopyable.h"

namespace glue_libbase {
class WaitMember: private Noncopyable {
 public:
  explicit WaitMember(int num): member_num_(num), mu_(), condvar_(mu_) { }

  void Done() {
    MutexLockGuard m(mu_);
	LOG_CHECK(member_num_ > 0, "Count of member sould be at least one");
	--member_num_;
	while (member_num_ != 0) {
		condvar_.Wait();
	}
	condvar_.NotifyAll();	
    LOG_INFO("WaitOver!");
  }
private:
	int	member_num_;		
	mutable MutexLock mu_;
	CondVar condvar_;
};
}  // namespace glue_libbase

#endif  // GLUE_LIBBASE_WAITMEMBER_H_
