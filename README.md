# rconc
## a simple RCON client written in C

rconc is basically a rewrite of [mcrcon](https://github.com/tiiffi/mcrcon), but with some differences:

- No Windows support
- Uses the GNU readline library, which allows for better input editing and command history
- Color!

### Building
rconc uses a standard Makefile. To build, just run `make`.  
To install into `/usr/local`, run `make install`.  

By default, rconc's Makefile will optimize for executable size.
To optimize for speed instead, set `OPTIMIZE=speed`:

```
make OPTIMIZE=speed
```

You can also disable optimization entirely:
```
make OPTIMIZE=none
```

On Arch Linux, you can use the [rconc-git AUR package](https://aur.archlinux.org/packages/rconc-git).

### Contribute
Any contributions can be sent to [dev@kasad.com](mailto:dev@kasad.com)

### To-Do
- Add tab completion for Minecraft commands
