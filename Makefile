all:
	cc clock.c -I /usr/local/include/ -L/usr/local/lib -lmonome -o clock
