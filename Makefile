
cosmopolitan-base-url = https://justine.lol/cosmopolitan
cosmopolitan-amalgamation = cosmopolitan-amalgamation-2.0.1.zip

cosmopolitan-download = $(cosmopolitan-base-url)/$(cosmopolitan-amalgamation)
cosmopolitan-dir = cosmopolitan


all: cosmosocks_server cosmosocks_server_ape

cosmosocks_server: cosmopolitan cosmosocks_server.c socks4.c common.c
	gcc -g -Os -static -nostdlib -nostdinc -fno-pie -no-pie -mno-red-zone \
		-fno-omit-frame-pointer -pg -mnop-mcount -mno-tls-direct-seg-refs \
		-o cosmosocks_server.com.dbg cosmosocks_server.c socks.c socks4.c socks5.c -fuse-ld=bfd -Wl,-T,$(cosmopolitan-dir)/ape.lds \
		-include $(cosmopolitan-dir)/cosmopolitan.h $(cosmopolitan-dir)/crt.o $(cosmopolitan-dir)/ape-no-modify-self.o $(cosmopolitan-dir)/cosmopolitan.a
	objcopy -S -O binary cosmosocks_server.com.dbg cosmosocks_server

cosmosocks_server_ape: cosmopolitan cosmosocks_server.c socks4.c common.c
	gcc -g -Os -static -nostdlib -nostdinc -fno-pie -no-pie -mno-red-zone \
		-fno-omit-frame-pointer -pg -mnop-mcount -mno-tls-direct-seg-refs \
		-o cosmosocks_server.com.dbg cosmosocks_server.c socks.c socks4.c socks5.c -fuse-ld=bfd -Wl,-T,$(cosmopolitan-dir)/ape.lds \
		-include $(cosmopolitan-dir)/cosmopolitan.h $(cosmopolitan-dir)/crt.o $(cosmopolitan-dir)/ape.o $(cosmopolitan-dir)/cosmopolitan.a
	objcopy -S -O binary cosmosocks_server.com.dbg cosmosocks_server_ape

cosmopolitan:
	[ -d cosmopolitan ] || wget $(cosmopolitan-download) && unzip -d $(cosmopolitan-dir) $(cosmopolitan-amalgamation)

clean:
	rm -f cosmosocks_server.com.dbg cosmosocks_server cosmosocks_server_ape.com.dbg cosmosocks_server_ape

distclean: clean
	rm -rf cosmopolitan $(cosmopolitan-amalgamation)

test:
	find tests/ -type f -print | xargs -I % bash % ${PWD}/cosmosocks_server 12345
	find tests/ -type f -print | xargs -I % bash % ${PWD}/cosmosocks_server_ape 12345
