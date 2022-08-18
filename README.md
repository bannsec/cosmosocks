# Overview
This is a silly socks program built without a purpose. Primarily, I just wanted to learn more about the socks protocol internals. To add a twist, my goal is to build it entirely in [Cosmopolitian libc](https://justine.lol/cosmopolitan). This should allow the socks program to run across any platform cosmopolitan can run on, which includes Windows, Linux, and MacOS.

# Running
The two binaries that are built are `cosmosocks_server.com` and `cosmosocks_server_ape.com`. The two binaries are functionally the same, but the former will unpack itself into a temporary directory while the later will dynamically overwrite itself.

# Building
All dependencies are provided in this repository. Building the program is simple as running `make`.

# TODO:

- [ ] Socks4 support
  - [x] Connect
  - [ ] Bind
  - [ ] Socks4a domain name resolve
- [ ] Socks5 support
  - [ ] Connect
  - [ ] Bind
  - [ ] UDP
  - [ ] Authentication: username/password