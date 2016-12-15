#ifndef GLUE_LIBBASE_NONCOPYABLE_H_
#define GLUE_LIBBASE_NONCOPYABLE_H_

namespace glue_libbase {
class Noncopyable 
{
 protected:
  Noncopyable() { }
  ~Noncopyable() { } 
 private:
  Noncopyable(const Noncopyable&);
  const Noncopyable& operator=(const Noncopyable&);
};
}

#endif	//GLUE_LIBBASE_NONCOPYABLE_H_
