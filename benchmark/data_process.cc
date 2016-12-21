#include <iostream>
#include <string>

int main()
{
	size_t cnt = 0;
	size_t sum = 0;
	std::string str;
	std::getline(std::cin, str);
	std::cout << str << ": ";
	while(std::getline(std::cin, str))
	{
		if(str.find("client") != std::string::npos)
		{
			if(cnt == 0) break;
			std::cout << static_cast<double>(sum)/static_cast<double>(cnt) << std::endl; 
			std::cout << str << ": ";
			sum = 0;
			cnt = 0;	
		}	
		else
		{
			sum += std::stoull(str);
			++cnt;	
		}
	}	
	std::cout << static_cast<double>(sum)/static_cast<double>(cnt) << std::endl; 

	return 0;
}

