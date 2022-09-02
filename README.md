[![Build Artifacts](https://github.com/bannsec/cosmosocks/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/bannsec/cosmosocks/actions/workflows/build.yml)

![Windows](https://github.com/bannsec/cosmosocks/raw/master/icons/windows.png)
![Apple](https://github.com/bannsec/cosmosocks/raw/master/icons/apple.png)
![Ubuntu](https://github.com/bannsec/cosmosocks/raw/master/icons/ubuntu.png)
![Android](https://github.com/bannsec/cosmosocks/raw/master/icons/android.png)
![OpenBSD](https://github.com/bannsec/cosmosocks/raw/master/icons/openbsd.png)
![NetBSD](https://github.com/bannsec/cosmosocks/raw/master/icons/netbsd.png)
![FreeBSD](https://github.com/bannsec/cosmosocks/raw/master/icons/freebsd.png)

# Overview
`Cosmosocks` is a socks server written entirely in [Cosmopolitian libc](https://justine.lol/cosmopolitan). This allows the socks program to run across any platform cosmopolitan can run on, which includes Windows, Linux, MacOS and more.

# Running
The two binaries that are built are `cosmosocks_server` and `cosmosocks_server_ape`. The two binaries are functionally the same, but the former will unpack itself into a temporary directory while the later will dynamically overwrite itself.

```bash
# Basic run with no arguments will bind global on port 1080
cosmosocks_server

# Specify a port to bind to
cosmosocks_server -p 8080

# Specify a username/password for authentication
cosmosocks_server -U user -P password
```

# Downloading
Release binaries are available on [GitHub](https://github.com/bannsec/cosmosocks/releases/latest).

# Building
All dependencies are provided in this repository. Building the program is simple as running `make`.
