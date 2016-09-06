#include <libbase/Heap.h>
#include <libbase/Debug.h>

#include <string.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>


using namespace libbase;

bool compare(const int& lhs, const int& rhs)
{
	return lhs > rhs;
}

void print(const std::vector<int> vec)
{
	std::ostream_iterator<int> output(std::cout, " ");
	std::copy(vec.begin(), vec.end(), output);
}

int main(int argc, char *argv[])
{
	int aryNum = atoi(argv[1]);
	std::vector<int> vec = {2, 1, -1, 234, 324, 34, 54, -123, 34, 34, 54, 568};
	std::vector<int> vecSorted;
	Heap<int> heap(true, aryNum, compare);

	std::cout << "input nums:\n";
	print(vec);
	for(auto elem : vec)
	{
		heap.insert(elem);
	}
	while(!heap.isEmpty())
		vecSorted.push_back(heap.topAndPop());

	std::cout << "\noutput nums:\n";
	print(vecSorted);

	std::cout << std::endl;
	return 0;

}


