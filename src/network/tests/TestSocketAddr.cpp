#include <network/SocketAddress.h>

#include <iostream>

using namespace network::csocket;
using namespace std;

typedef SocketAddress::AddrType AddrType;
typedef SocketAddress::AddrType4 AddrType4;

int main()
{
	//local address
	SocketAddress sa;
		
	cout << sa.toString() << "\n";
	cout << "addr  get length: " << sa.getAddrLength() << "\n";
	cout << "addr4 length: " << sizeof(AddrType4) << "\n";
	cout << "addr  length: " << sizeof(AddrType) << "\n";
	return 0;		
}


