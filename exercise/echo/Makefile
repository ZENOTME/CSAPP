echoservers:
	gcc echoservers.c -lcsapp -lpthread -o echoservers
echoservert:
	gcc echoservert.c -lcsapp -lpthread -o echoservert
echoservert-pre:
	gcc echoservert-pre.c sbuf.c sbuf.h -lcsapp -lpthread -o echoserver-pre

clean:
	rm -f echoservers echoservert echoserver-pre
