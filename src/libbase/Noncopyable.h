#ifndef LIBBASE_NONCOPYABLE_H
#define LIBBASE_NONCOPYABLE_H

namespace libbase 
{

class Noncopyable 
{
protected:
	Noncopyable() {}
	~Noncopyable() {}
private:
	Noncopyable(const Noncopyable&);
	const Noncopyable& operator=(const Noncopyable&);
};

}
#endif		//LIBBASE_NONCOPYABLE_H
