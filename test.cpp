#include "httpParser.h"
#include <iostream>
#include <cstring>
using namespace std;

int main(){
	httpParser p;
	char in[]="GET / HTTP/1.0\r\nHost: intranet.iitg.ernet.in\r\nUser-Agent: HTTPTool/1.0\r\n\r\n";
	char *t = p.parseHeaders(in,strlen(in));
	cout<<t<<endl;
	cout<<p.method<<endl;
	cout<<p.requestURI<<endl;
	cout<<p.httpVer<<endl;
	p.print();

}