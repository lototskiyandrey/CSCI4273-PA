make:
	-gcc udp_client.c -o client -Werror -Wall -Wextra
	-gcc udp_server.c -o server -Werror -Wall -Wextra
clean:
	rm client
	rm server