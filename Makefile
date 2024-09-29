make:
	-gcc udp_client.c -o client -Werror
	-gcc udp_server.c -o server -Werror
clean:
	rm client
	rm server