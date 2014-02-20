/*
 * httpParser.h
 *
 *  Created on: 30 Jan 2014
 *      Author: Harshil Lodhi
 *      Networks Assignmnet 2 - HTTP Proxy Server
 *
 */

#include <string>
#include <vector>
#include <list>
using namespace std;

#define GET 1
#define POST 2
#define HEAD 3
#define PUT 4
#define DELETE 5
#define OPTIONS 6
#define CONNECT 7

/*
	Exception class to throw exceptions.
*/
class ParseException : public std::exception
{
public:
  ParseException (const std::string &reason) : m_reason (reason) { }
  virtual ~ParseException () throw () { }
  virtual const char* what() const throw ()
  { return m_reason.c_str (); }
private:
  std::string m_reason;
};


/*
	Structure representing httpHeader field
	key: value

	Comparision is done by keys.
*/
struct httpHeader{
		string key,value;
		httpHeader(string &key_, string &value_){
			key = key_;
			value = value_;
		}

		bool operator ==(const string &key_){
			return key==key_;
		}
		bool operator <(const string &key_){
			return this->key < key_;
		}
	};

/*
	A wrapper class to various http parsing and header functions.
*/
class httpParser{
private:

	list<httpHeader> listHeader; // Contains all the headers of a request
	string httpVer;				// the HTTP version of the request
	string msgBody;				// The message body of a HTTP Request
public:

	int method;					// Method of an HTTP request
	string requestURI;			// The request URI without hostname
	
	httpParser();
	/**
	 * Parses the request in buffer and fills listHeader
	 * @param  buffer
	 * @param  size
	 * @return const int
	 */				
	const int parseHeaders(const char *buffer, size_t size);

	/**
	 * Gives the total length of the request.
	 * @return size_t
	 */
	size_t getTotalLength();


	char *formatHeaders(char *buffer);

	/**
	 * To add a new header filed and its value into the request
	 * @param key
	 * @param value
	 */
	void addHeader(string &key, string &value);


	/**
	 * To remove a header using key
	 * @param key
	 */
	void removeHeader(string &key);


	/**
	 * Modifies the header value
	 * If the header is not there, it adds it to the request.
	 * @param key
	 * @param value
	 */
	void modifyHeader(string &key, string &value);


	/**
	 * Returns the value of the header
	 * @param  key
	 * @return string
	 */
	string findHeader(string &key);


	/**
	 * Prints all the headers. For debug purposes.
	 */
	void print();


	/**
	 * Returns a string containing one header per line which can be appended to a request message
	 * @return string
	 */
	string getAllHeadersFormatted();


	/**
	 * Returns the method no. that the request is using
	 * @return int
	 */
	int getMethod();


	/**
	 * Returns requestURI without hostname
	 * @return string
	 */
	string getRequestURI();


	/**
	 * Returns the http version that the request is using
	 * @return string
	 */
	string gethttpVer();

	/**
	 * Sets the message body in a request
	 * @param msg
	 */
	void setMessageBody(string &msg);
	/**
	 * Returns the message body
	 * @return string
	 */
	string getMessageBody();


};
