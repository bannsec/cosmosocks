Since this binary is based on cosmopolitan libc, debugging can be a bit different than usual. If you're running into issues, start off by re-building in debug mode:

```bash
make COSMO=amalgamation-dbg
```

Then the following commands may be useful:

```bash
# To get strace type output
./cosmosocks_server --strace

# To get ltrace type output
./cosmosocks_server --ftrace

# To step through with debugging information and source code:
gdb -ex 'add-symbol-file cosmosocks_server.com.dbg 0x401000' ./cosmosocks_server
```

See the help page for cosmopolitan libc for more information: https://justine.lol/cosmopolitan/windows-debugging.html
