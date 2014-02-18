/*
 * httpParser.cpp
 *
 *  Created on: 30 Jan 2014
 *      Author: harshil
 */
#include "httpParser.h"
#include <list>
#include <algorithm>
#include <vector>
#include <cstring>
#include <iostream>
#include <sstream>

#define PR(x) cout<< #x <<" = "<<x<<endl;
using namespace std;

static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}


httpParser::httpParser ()
{
}

const int httpParser::parseHeaders (const char *buffer, size_t size)
{
  const char *curPos = buffer;
  string key;
  string value;
  cout<<"-           - - -- - - BUFFER"<<endl;

  PR(buffer)
  //Checking for method
  char *end = (char *)memmem (curPos, size - (curPos-buffer), "\r\n", 2);
  if(end ==0 ){
    throw ParseException("Request Line Doesnt end with \\r\\n");
  }
  string requestLine(curPos,end - curPos);
  stringstream ss(requestLine);
  string token;
  ss>>token;
  if(token == "GET"){
    method = GET;
  }else if (token== "POST" ){
    method = POST;
  }else if( token== "DELETE" ){
    method = DELETE;
  }else if(token== "PUT" ){
    method = PUT;
  }else if(token== "OPTIONS" ){
    method = OPTIONS;
  }else if(token== "CONNECT" ){
    method = CONNECT;
  }else if(token== "HEAD"){
    method = HEAD;
  }else{
	cerr<<"Unknown Method or Improper msg format"<<endl;
	return -1;
  }
  ss>>token;
  requestURI = token;
  ss>>token;
  httpVer = token;
  curPos = end + 2;
  while (((size_t)(curPos-buffer) <= size-2) &&
         (*curPos != '\r' && *(curPos+1) != '\n'))
    {
      char *endline = (char *)memmem (curPos, size - (curPos-buffer), "\r\n", 2);
      if (endline == 0)
        {
    	  cerr<<"Header line does end with \\r\\n"<<endl;
    	  	return -1;

        }
      
      if (*curPos == ' ' || *curPos == '\t') // multi-line header
        {
          if (key == "")
          {
        	  cerr<<"Multi-line header without actual header"<<endl;
        	      	  	return -1;
          }


          
          string newline (curPos, endline-curPos);
          // TRACE ("Multi-line header: " << value << " + " << newline);

          // reusing key from previous iteration
          value += "\r\n" + newline;
          modifyHeader (key, value);
        }
      else
        {
          char *header_key = (char*)memmem (curPos, endline - curPos, ":", 1);

          if (header_key == 0)
            {
        	  cerr<<"HTTP header doesn't contain ':'"<<endl;
        	  return -1;

            }

          key = string (curPos, header_key-curPos);
          value = string (header_key+1, endline - header_key - 1);
          ltrim (value); // remove any leading spaces if present
      
          // TRACE ("Key: [" << key << "], value: [" << value << "]");

          modifyHeader (key, value);
        }
      
      curPos = endline + 2;
    }

  // TRACE ("Left: " << (int)(curPos-buffer) << ", size: " << size);
  if (static_cast<size_t> (curPos-buffer+2) <= size)
    {
      curPos += 2; // skip '\r\n'
    }
  else
    {
	  cerr<<"Parsed buffer does not contain \\r\\n"<<endl;
	          	  return -1;

    }

    // modifying request URI a/c to the speciations and adding host header if necessary
  string host = "Host";
  string http = "http://";

  if(requestURI[0] == '/' && httpParser::findHeader(host) != "" ){
    //everything is fine
  }else {
	  string requestURIcmp = requestURI.substr(0,http.size());
	  std::transform(requestURIcmp.begin(), requestURIcmp.end(), requestURIcmp.begin(), ::tolower);
	  if(requestURIcmp.compare(0,http.size(),http) == 0){
		  requestURI = requestURI.substr(http.size());
		  	  int pos = requestURI.find("/");
		  	  int lenofhost = pos;
		  	  string hostHeader = requestURI.substr(0,lenofhost);
		  	  PR(hostHeader)
		  	  requestURI = requestURI.substr(pos);
		  	  PR(requestURI)
		  	  modifyHeader(host,hostHeader);
	  }

  }

  return 0;
}

size_t
httpParser::getTotalLength () {
  size_t len = 0;
  for (std::list<httpHeader>::iterator header = listHeader.begin ();
       header != listHeader.end ();
       header++)
    {
      len += header->key.size () + 2/*: */+ header->value.size () + 2/*\r\n*/;
    }
  
  return len;
}

char*
httpParser::formatHeaders (char *buffer) {
  char *bufLastPos = buffer;
  
  for (std::list<httpHeader>::iterator header = listHeader.begin ();
       header != listHeader.end ();
       header++)
    {
      bufLastPos = stpncpy (bufLastPos, header->key.c_str (), header->key.size ());
      bufLastPos = stpncpy (bufLastPos, ": ", 2);
      bufLastPos = stpncpy (bufLastPos, header->value.c_str (), header->value.size ());
      bufLastPos = stpncpy (bufLastPos, "\r\n", 2);
    }
  
  return bufLastPos;
}


void
httpParser::addHeader (std::string &key, std::string &value)
{
  listHeader.push_back (httpHeader (key, value));
}

void httpParser::removeHeader (std::string &key)
{
  std::list<httpHeader>::iterator item = std::find (listHeader.begin (), listHeader.end (), key);
  if (item != listHeader.end ())
    listHeader.erase(item);
}

void httpParser::modifyHeader (std::string &key, std::string &value)
{
  std::list<httpHeader>::iterator item = std::find (listHeader.begin (), listHeader.end (), key);
  if (item != listHeader.end ())
    item->value = value;
  else
    addHeader (key, value);
}

std::string
httpParser::findHeader (std::string &key)
{
  std::list<httpHeader>::iterator item = std::find (listHeader.begin (), listHeader.end (), key);
  if (item != listHeader.end ())
    return item->value;
  else
    return "";
}

void httpParser::print(){
  for (std::list<httpHeader>::iterator header = listHeader.begin ();
       header != listHeader.end ();
       header++){
    cout<<header->key<<" : "<<header->value<<endl;
  }
}

string httpParser::getAllHeadersFormatted(){
	stringstream ss;
	for (std::list<httpHeader>::iterator header = listHeader.begin ();
	       header != listHeader.end ();
	       header++){
	    ss<<header->key<<": "<<header->value<<"\r\n";
	  }
	return ss.str();
}

int httpParser::getMethod(){
	return method;
}

string  httpParser::getRequestURI(){
	return requestURI;
}

string httpParser::gethttpVer(){
	return httpVer;
}
string httpParser::getMessageBody(){
	return msgBody;
}

void httpParser::setMessageBody(string& msg){
	msgBody = msg;
}




