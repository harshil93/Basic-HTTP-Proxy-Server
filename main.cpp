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
#include <errno.h>
#include "httpParser.h"
#include "httpMsg.h" // error messages
using namespace std;

#define PR(x) cout << #x " = " << x << "\n"; // for debugging purposes

#define REMOTE_SERVER_PORT "80" // default remote server port
#define BACKLOG 100             // No. of backlog reqeusts
#define BUFSIZE 2048			// BufferSize


/**
 * server_listen - bind to the supplied port and listen
 * @param  char* port - a string
 * @return the fd if no error otherwise <0 value which indicates error
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

/**
 * A function wrapper to wrap both IPv4 and IPv6
 * @param  struct sockaddr *sa
 */
void *get_in_addr (struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 * Accepts a client connection. The server fd is passed as a para
 * @param  server_fd
 * @return client_fd
 */
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
	tv.tv_sec = 120;  /* 15 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors
	setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	return client_fd;
}

/**
 * Creates socket, connects to remote host
 * @param const char *host  - Host's domain name or IP address 
 * @param const char *port - The port to which we have to make connection. 
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
    fprintf(stderr, "Cannot get address info\n");
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
 * A wrapper function on send() socket all which tries to send all the data that is in the buffer
 * @param  int socket
 * @param  const void *buffer
 * @param  size_t length
 * @return
 */
int send_all(int socket,const void *buffer, size_t length) {
    size_t i = 0;
    for (i = 0; i < length;){
    	int bytesSent = send(socket, buffer, length - i,MSG_NOSIGNAL);
    	if(bytesSent==-1){
    		return errno;
    	}else{
    		i+=bytesSent;
    	}
    }
    return 0;
}

/**
 * Gets all data from remote host and forwards it to the client
 * @param int remote_fd - the remote fd from which we have to fetch data
 * @param string result -  the received data is also stored in result
 * @param int sendfd - the fd of the original client to which the data is to be forwarded
 * @returns 0 on success, <0 on failure
 */
int get_data_from_host_and_send_to_client (int remote_fd, string &result,int sendfd)
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
      cout<<"error"<<endl;
      return -1;
    }

    // If we didn't recieve anything, we hit the end

    else if (num_recv == 0)
      break;
    // Append the buffer to the response if we got something
    result+= string(res_buf,res_buf+num_recv);
    if(send_all(sendfd,res_buf,num_recv) !=0){
    	//cerr<<"Error No = "<<errno<<endl;
    	close(sendfd);
    	close(remote_fd);
    	return errno;
    }

  }

  close(sendfd);
  close(remote_fd);
  return 0;
}

/**
 * Creates a request using the httpParser Object and sends it to clientfd
 * @param  int fd - socket descriptor
 * @param  httpParser parser - the httpParser object containing info regarding the request
 * @return 0 on success and errno in case of error
 */
int sendRequest(int fd, httpParser &parser){
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
	if(send_all(fd,msg,requestMsg.size()) !=0){
		close(fd);
		return errno;
	}
	return 0;

}

/**
 * handle Request handles the incoming requests specified by the in_fd socket descriptor
 * It reads the total request and appropriate creates the httpParser Object which can be used later for different purposes
 * @param int in_fd
 */
void handleRequest(int in_fd){
	char buf[10001];
	int bytesRead=0;
	string requestMsg="";
	string remaining="";
	while((bytesRead = recv(in_fd,buf,10000,0)) >0){
		string curMsg = string(buf,buf+bytesRead);
		requestMsg +=curMsg;

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
	cout<<requestMsg<<endl;
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

	parser.setMessageBody(remaining);
	if(parser.getMethod() == GET){
		string temp = "Host";
		string hoststr = parser.findHeader(temp);
		const char *host = hoststr.c_str();
		if(host==NULL || parser.findHeader(temp)==""){
			fprintf(stderr,"%s\n","BAD REQ");
			send_all(in_fd,BAD_REQUEST,strlen(BAD_REQUEST));
			close(in_fd);
			return;
		}

		int clientfd = make_client_connection(host,REMOTE_SERVER_PORT);
		if(clientfd == -1){
			close(in_fd);
			close(clientfd);
			return ;
		}
		sendRequest(clientfd,parser);
		string response;
		get_data_from_host_and_send_to_client(clientfd,response,in_fd);


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
			cout<<"Handling Request "<<endl;
			handleRequest(in_fd);
		}

	}
	return 0;

}
