#ifndef GLUE_HTTPD_STRINGUTIL_H
#define GLUE_HTTPD_STRINGUTIL_H

#include <vector>
#include <string>
#include <algorithm>

namespace httpd {
  std::vector<std::string> SliceStr(const std::string& str, const std::string& target);
  std::string RemoveSpace(const std::string& str);
  std::vector<std::string> SliceStrClearly(const std::string& str, const std::string& target);
  bool IsNumStr(const std::string& num_str);
} // namespace httpd
#endif // GLUE_HTTPD_STRINGUTIL_H_
