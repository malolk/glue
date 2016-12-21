#include "httpd/stringutil.h"

namespace glue_httpd {
std::vector<std::string> SliceStr(const std::string& str, const std::string& target) {
  std::string::size_type pos = 0, rt_pos = 0;
  std::vector<std::string> segments;
  std::vector<std::string::size_type> target_pos;
  while (pos < str.size()) {
  	rt_pos = str.find(target, pos);
  	if (rt_pos == std::string::npos) {
  	  break;
    }
  	target_pos.push_back(rt_pos);
  	pos = rt_pos + target.size();
  }

  if (target_pos.size() > 0) {
  	std::string::size_type start_pos = 0, end_pos = target_pos[0];
  	for (std::vector<std::string::size_type>::const_iterator it = target_pos.cbegin(); it != target_pos.cend(); ++it) {
  	  if (start_pos != *it) {
  	    segments.push_back(str.substr(start_pos, *it - start_pos));
      }
  	  start_pos = *it + target.size();
  	}       
  	if (start_pos < str.size()) {
  	  segments.push_back(str.substr(start_pos, str.size() - start_pos));
    }
  }    
  return segments;
}

std::string RemoveSpace(const std::string& str) {
  std::string::const_iterator beg = std::find_if(str.cbegin(), str.cend(),
  		                            [](const char ch) -> bool { return (ch != ' '); });
  if (beg == str.cend()) {
  	return std::string();
  }
  std::string::const_reverse_iterator end = std::find_if(str.crbegin(), str.crend(),
  		                                    [](const char ch) -> bool { return (ch != ' '); });
  return std::string(beg, str.cend() - (end - str.crbegin()));
}

std::vector<std::string> SliceStrClearly(const std::string& str, const std::string& target) {
  std::vector<std::string> segments = SliceStr(str, target);
  std::vector<std::string> ret_segments;
  for (std::vector<std::string>::const_iterator it = segments.cbegin(); it != segments.cend(); ++it) {
  	std::string ret_str = RemoveSpace(*it);
  	if (!ret_str.empty()) {
  		ret_segments.push_back(ret_str);
    }
  }
  return ret_segments;
}

bool IsNumStr(const std::string& num_str) {
  for (std::string::const_iterator it = num_str.cbegin(); it != num_str.cend(); ++it) {
  	if (!std::isdigit(*it)) {
  	  return false;
    }
  }
  return true;
}
} // namespace glue_httpd
