#include <libbase/TimeStamp.h>

#include <iostream>

using namespace libbase;
int main()
{
	TimeStamp tm1, tm2;
	std::cout << tm1.bePretty() << std::endl;
	std::cout << tm2.bePretty() << std::endl;
	
	std::cout << (tm1 < tm2) << std::endl;
	return 0;
}

