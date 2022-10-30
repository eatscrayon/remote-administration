# remote-administration

A collection of simple bindshells and reverse shells written in C

The bindshell will accept multiple connections.
```Usage: ./bindshell [PORT]```

The reverse shell will attempt to make a connection once every 30 seconds.
```Usage: ./reverse-shell [IP] [PORT]```

If you run them at the same time the universe will implode.

UPDATE: Arguments are now hidden from ps.

UPDATE: The xor and rc4 versions of the reverse shells are in the folders.
