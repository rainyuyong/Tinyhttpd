all: httpd httpclient

httpd: httpd.c
	gcc -g -W -Wall  -lpthread -o httpd httpd.c
httpclient: httpclient.c
	gcc -g -W -Wall  -lpthread -o httpclient httpclient.c

clean:
	rm httpd
