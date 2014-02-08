/*
 * httpParser.h
 *
 *  Created on: 30 Jan 2014
 *      Author: harshil
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

/**
 * Exception that will be thrown when parsing cannot be performed
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
class httpParser{
private:
	list<httpHeader> listHeader;
	string httpVer;
	string msgBody;
public:

	int method;
	string requestURI;
	httpParser();
	const int parseHeaders(const char *buffer, size_t size);
	size_t getTotalLength();
	char *formatHeaders(char *buffer);
	void addHeader(string &key, string &value);
	void removeHeader(string &key);
	void modifyHeader(string &key, string &value);
	string findHeader(string &key);
	void print();
	string getAllHeadersFormatted();
	int getMethod();
	string getRequestURI();
	string gethttpVer();
	void setMessageBody(string &msg);
	string getMessageBody();


};
