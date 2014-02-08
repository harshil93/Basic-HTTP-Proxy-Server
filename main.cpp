#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h> // inet_pton
#include <unistd.h> // close()
#include "httpParser.h"
#include "httpMsg.h"
using namespace std;
#define PR(x) cout << #x " = " << x << "\n";

#define REMOTE_SERVER_PORT "80"
#define BACKLOG 100
#define BUFSIZE 2048

/*
* server_listen - bind to the supplied port and listen
* port is a string
* returns the fd if no error otherwise <0 value which indicates error
*/
int server_listen(char *port){

	// Create address structs
	struct addrinfo hints, *res;
	int sock_fd;

	// Load up address structs
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int addr_status = getaddrinfo(NULL, port, &hints, &res);
	if (addr_status != 0)
	{
	  fprintf(stderr, "Cannot get info\n");
	  return -1;
	}

	// Loop through results, connect to first one we can
	struct addrinfo *p;
	for (p = res; p != NULL; p = p->ai_next)
	{
	  // Create the socket
	  sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	  if (sock_fd < 0)
	  {
	    perror("server: cannot open socket");
	    continue;
	  }

	  // Set socket options
	  int yes = 1;
	  int opt_status = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	  if (opt_status == -1)
	  {
	    perror("server: setsockopt");
	    exit(1);
	  }

	  // Bind the socket to the port
	  int bind_status = bind(sock_fd, res->ai_addr, res->ai_addrlen);
	  if (bind_status != 0)
	  {
	    close(sock_fd);
	    perror("server: Cannot bind socket");
	    continue;
	  }

	  // Bind the first one we can
	  break;
	}

	// No binds happened
	if (p == NULL)
	{
	  fprintf(stderr, "server: failed to bind\n");
	  return -2;
	}

	// Don't need the structure with address info any more
	freeaddrinfo(res);

	// Start listening
	if (listen(sock_fd, BACKLOG) == -1) {
	  perror("listen");
	  exit(1);
	}

	return sock_fd;
}
void *get_in_addr (struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int accept_connection(int server_fd){
	struct sockaddr_storage their_addr; // connector's address information
	char s[INET6_ADDRSTRLEN];
	socklen_t sin_size = sizeof their_addr;

	// Accept connections
	int client_fd = accept(server_fd, (struct sockaddr *)&their_addr, &sin_size);
	if (client_fd == -1)
	{
	  perror("accept");
	  return -1;
	}

	// Print out IP address
	inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
	printf("server: got connection from %s\n", s);
	// Setting Timeout
	struct timeval tv;
	tv.tv_sec = 10;  /* 30 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors
	setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	return client_fd;
}
// get sockaddr, IPv4 or IPv6:


/**
 * @brief Creates socket, connects to remote host
 * @returns fd of socket, <0 if error
 */
int make_client_connection (const char *host, const char *port)
{
  // Create address structs
  struct addrinfo hints, *res;
  int sock_fd;

  // Load up address structs
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  //fprintf(stderr, "%s %s\n", host, port);

  int addr_status = getaddrinfo(host, port, &hints, &res);
  if (addr_status != 0)
  {
    fprintf(stderr, "Cannot get info\n");
    return -1;
  }

  // Loop through results, connect to first one we can
  struct addrinfo *p;
  for (p = res; p != NULL; p = p->ai_next)
  {
    // Create the socket
    sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_fd < 0)
    {
      perror("client: cannot open socket");
      continue;
    }

    // Make connection
    int connect_status = connect(sock_fd, p->ai_addr, p->ai_addrlen);
    if (connect_status < 0)
    {
      close(sock_fd);
      perror("client: connect");
      continue;
    }

    // Bind the first one we can
    break;
  }

  // No binds happened
  if (p == NULL)
  {
    fprintf(stderr, "client: failed to connect\n");
    return -2;
  }

  // Print out IP address
  char s[INET6_ADDRSTRLEN];
  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
  //fprintf(stderr, "client: connecting to %s\n", s);

  // Don't need the structure with address info any more
  freeaddrinfo(res);

  return sock_fd;
}

/**
 * @brief Gets all data from remote host
 * @returns 0 on success, <0 on failure
 * @param result The string that the data is appended to
 */
int get_data_from_host (int remote_fd, string &result)
{
  // Loop until we get the last segment? packet?
  for (;;)
  {
    char res_buf[BUFSIZE];

    // Get data from remote
    int num_recv = recv(remote_fd, res_buf, sizeof(res_buf), 0);
    if (num_recv < 0)
    {
      perror("recv");
      return -1;
    }

    // If we didn't recieve anything, we hit the end
    else if (num_recv == 0)
      break;

    // Append the buffer to the response if we got something
    result.append(res_buf, num_recv);
  }
  return 0;
}
void send_all(int socket,const void *buffer, size_t length) {
    size_t i = 0;
    for (i = 0; i < length; i += send(socket, buffer, length - i,0));
}
int sendRequest(int clientfd, httpParser &parser){
	string requestMsg = "";
	switch(parser.getMethod()){
		case GET:
			requestMsg+="GET ";
			break;
		// add more cases when you will add more methods
	}
	requestMsg+=parser.getRequestURI()+" "+parser.gethttpVer()+"\r\n";
	requestMsg+= parser.getAllHeadersFormatted();
	requestMsg+= "\r\n" + parser.getMessageBody();
	const char *msg = requestMsg.c_str();
	send_all(clientfd,msg,requestMsg.size());
	return 0;

}

void handleRequest(int in_fd){
	char buf[10001];
	int bytesRead=0;
	string requestMsg="";
	string remaining="";
	while((bytesRead = recv(in_fd,buf,10000,0)) >0){
		string curMsg = string(buf,buf+bytesRead);
		requestMsg +=curMsg;
		cout<<curMsg;
		int pos;
		if((pos = requestMsg.find("\r\n\r\n")) !=  string::npos ){
			requestMsg = requestMsg.substr(0,pos+4);
			remaining = requestMsg.substr(pos+4);
			break;
		}

	}

	if(requestMsg.size() == 0){
		fprintf(stderr,"%s\n","Request Msg size is 0");
		close(in_fd);
		return;
	}

	cout<<endl<<endl;
	httpParser parser;
	const char *ptr = requestMsg.c_str();
	int pstatus = parser.parseHeaders(ptr,requestMsg.size());
	if(pstatus<0){
		//send wrong message format error;
		fprintf(stderr,"%s\n","BAD REQ");
		send_all(in_fd,BAD_REQUEST,strlen(BAD_REQUEST));
		close(in_fd);
		return;
	}
	string contentlen = "Content-Length";
	string contentlenval = parser.findHeader(contentlen);
	if(contentlenval != ""){
		cout<<"Content Header Len = "<<contentlenval<<endl;
		// receive the body of the message
		int len = atoi(contentlenval.c_str());
		len -= remaining.size();
		while(len!=0){
			bytesRead = recv(in_fd,buf,10000,0);
			if(bytesRead<=0) break;
			remaining += string(buf,buf+bytesRead);
			len-=bytesRead;
		}
	}
	cout<<"Remaining"<<endl;
	cout<<remaining<<endl;
	parser.setMessageBody(remaining);
	if(parser.getMethod() == GET){
		string temp = "Host";
		const char *host = parser.findHeader(temp).c_str();
		int clientfd = make_client_connection(host,REMOTE_SERVER_PORT);
		sendRequest(clientfd,parser);
		cout<<"request sent"<<endl;
		string response;
		get_data_from_host(clientfd,response);

		const char *res = response.c_str();
		send_all(in_fd,res,response.size());
		close(in_fd);
		close(clientfd);
	}else{
		fprintf(stderr,"%s\n","NOT IMPLEMENTED");
		send_all(in_fd,NOT_IMPLEMENTED,strlen(NOT_IMPLEMENTED));
		close(in_fd);
		// send message that its not implemented.
	}

}

int main(int argc, char **argv){
	if(argc != 2){
		cout<<"The format is "<<"./a.out <port>"<<endl;
		return 0;
	}

	// make server bind and listen to the supplied port
	int server_fd;
	if((server_fd = server_listen(argv[1])) <0 ){
		fprintf(stderr, "%s\n", "Error listening to given port");
		return 0;
	}

	cout<<"Server Listening at "<<argv[1]<<endl;
	// now start accept incoming connections:

	while(1){
		int in_fd;
		if((in_fd = accept_connection(server_fd)) <0){
			fprintf(stderr, "%s\n", "Error Accepting Connections");
		}else{
			cout<<"Handling Request "<<in_fd<<endl;
			handleRequest(in_fd);
		}

	}
	return 0;

}
