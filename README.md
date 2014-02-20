http-proxy-server
=================

A Very Basic Proxy Server. (Single Threaded)

To Compile:
	$ make

To Run:
	$ ./main <port no.>
	eg. $ ./main 9000


To use telnet:
	$ telnet 127.0.0.1 9000

IMPORTANT POINTS TO REMEMBER:
============================

Configuring a Web Browser to Use a Proxy:
-----------------------------------------
Firefox:

Select Tools->Options (or Edit->Preferences) from the menu.
Click on the 'Advanced' icon in the Options dialog.
Select the 'Network' tab, and click on 'Settings' in the 'Connections' area.
Select 'Manual Proxy Configuration' from the options available. In the boxes, enter the hostname and port where proxy program is running.

Configuring Firefox to use HTTP/1.0
-----------------------------------
Because Firefox defaults to using HTTP/1.1 and your proxy speaks HTTP/1.0, there are a couple of minor changes that need to be made to Firefox's configuration. Fortunately, Firefox is smart enough to know when it is connecting through a proxy, and has a few special configuration keys that can be used to tweak the browser's behavior.

Type 'about:config' in the title bar.
In the search/filter bar, type 'network.http.proxy'
You should see three keys: network.http.proxy.keepalive, network.http.proxy.pipelining, and network.http.proxy.version.
Set keepalive to false. Set version to 1.0. Make sure that pipelining is set to false.


Sample Request
==============

After running the proxy server at the given port (say 9000), open a new terminal and type

$ telnet 127.0.0.1 9000
to connect to proxy server
then type in the following sample request

GET /cseintranet/ HTTP/1.0
Host: jatinga.iitg.ernet.in

<two enters to end the request>

Output will be in the terminal

Contact: Harshil Lodhi - +91 - 8876858859 (incase of problem in compiling or running)
