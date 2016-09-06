#include <libbase/StringUtil.h>

namespace libbase
{
std::vector<std::string> sliceStr(const std::string& str, const std::string& target)
{
	std::string::size_type pos = 0, rtPos = 0;
	std::vector<std::string> segments;
	std::vector<std::string::size_type> targetPos;
	while (pos < str.size())
	{
		rtPos = str.find(target, pos);
		if (rtPos == std::string::npos)
			break;
		targetPos.push_back(rtPos);
		pos = rtPos + target.size();
	}

	if (targetPos.size() > 0)
	{
		std::string::size_type startPos = 0, endPos = targetPos[0];
		for(std::vector<std::string::size_type>::const_iterator it = targetPos.cbegin(); it != targetPos.cend(); ++it)
		{
			if (startPos != *it)
				segments.push_back(str.substr(startPos, *it - startPos));
			startPos = *it + target.size();
		}       
		if (startPos < str.size())
			segments.push_back(str.substr(startPos, str.size() - startPos));
	}    
	return segments;
}

std::string removeSpace(const std::string& str)
{
	std::string::const_iterator beg = std::find_if(str.cbegin(), str.cend(),
			[](const char ch) -> bool { return (ch != ' '); });
	if (beg == str.cend())
		return std::string();
	std::string::const_reverse_iterator end = std::find_if(str.crbegin(), str.crend(),
			[](const char ch) -> bool { return (ch != ' '); });
	return std::string(beg, str.cend() - (end - str.crbegin()));
}

std::vector<std::string> sliceStrClearly(const std::string& str, const std::string& target)
{
	std::vector<std::string> segments = sliceStr(str, target);
	std::vector<std::string> retSegments;
	for(std::vector<std::string>::const_iterator it = segments.cbegin(); 
	it != segments.cend(); ++it)
	{
		std::string retStr = removeSpace(*it);
		if (!retStr.empty())
			retSegments.push_back(retStr);
	}

	return retSegments;
}

bool isNumStr(const std::string& numStr)
{
	for(std::string::const_iterator it = numStr.cbegin(); 
	it != numStr.cend(); ++it)
	{
		if (!std::isdigit(*it))
			return false;
	}
	return true;
}

}
