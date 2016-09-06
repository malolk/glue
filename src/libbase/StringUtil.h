#ifndef LIBBASE_STRINGUTIL_H
#define LIBBASE_STRINGUTIL_H

#include <vector>
#include <string>
#include <algorithm>

namespace libbase
{
	std::vector<std::string> sliceStr(const std::string& str, const std::string& target);
	std::string removeSpace(const std::string& str);
	std::vector<std::string> sliceStrClearly(const std::string& str, const std::string& target);
	bool isNumStr(const std::string& numStr);
}
#endif 
