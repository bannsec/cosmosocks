
# Basic makefile for the project
# Default build all should compile cosmosocks_server.c

all: cosmosocks_server cosmosocks_server_ape

cosmosocks_server: cosmosocks_server.c socks4.c common.c
	gcc -g -Os -static -nostdlib -nostdinc -fno-pie -no-pie -mno-red-zone \
		-fno-omit-frame-pointer -pg -mnop-mcount -mno-tls-direct-seg-refs \
		-o cosmosocks_server.com.dbg cosmosocks_server.c socks.c socks4.c -fuse-ld=bfd -Wl,-T,ape.lds \
		-include cosmopolitan.h crt.o ape-no-modify-self.o cosmopolitan.a
	objcopy -S -O binary cosmosocks_server.com.dbg cosmosocks_server.com

cosmosocks_server_ape: cosmosocks_server.c socks4.c common.c
	gcc -g -Os -static -nostdlib -nostdinc -fno-pie -no-pie -mno-red-zone \
		-fno-omit-frame-pointer -pg -mnop-mcount -mno-tls-direct-seg-refs \
		-o cosmosocks_server.com.dbg cosmosocks_server.c socks.c socks4.c -fuse-ld=bfd -Wl,-T,ape.lds \
		-include cosmopolitan.h crt.o ape.o cosmopolitan.a
	objcopy -S -O binary cosmosocks_server.com.dbg cosmosocks_server_ape.com


clean:
	rm -f cosmosocks_server.com.dbg cosmosocks_server.com cosmosocks_server_ape.com.dbg cosmosocks_server_ape.com

test:
	find tests/ -type f -print | xargs -I % bash % ${PWD}/cosmosocks_server.com 12345
	find tests/ -type f -print | xargs -I % bash % ${PWD}/cosmosocks_server_ape.com 12345