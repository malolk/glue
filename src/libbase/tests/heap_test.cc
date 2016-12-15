#include "../heap.h"
#include "../logger.h"

#include <string.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>

bool CompFunc(const int& lhs, const int& rhs) {
  return lhs < rhs;
}

void Print(const std::vector<int> vec) {
  std::string ret;
  for (int i = 0; i < vec.size(); ++i) {
    ret += std::to_string(vec[i]) + std::string(" ");   
  }
  LOG_INFO("Vec contents: %s", ret.c_str());
}

void TestCase(int ary_num, const std::vector<int>& vec) {
  std::vector<int> sorted_vec;
  glue_libbase::Heap<int> heap(true, ary_num, CompFunc);
  Print(vec);
  for (auto elem : vec) {
    heap.Insert(elem);
  }
  while (!heap.Empty()) {
    sorted_vec.push_back(heap.TopAndPop());
  }
  Print(sorted_vec);
}

int main() { 
  std::vector<int> vec0 = {1};
  std::vector<int> vec = {2, 1, -1, 234, 324, 34, 54, -123, 34, 34, 54, 568};
  TestCase(2, vec0);
  TestCase(2, vec);
  TestCase(4, vec0);
  TestCase(4, vec);
  return 0;
}


