CC=cl
CFLAGS=/D_WIN32

cl /c client.c
cl /c version.c 

link client.obj version.obj /DLL /out:distdb.dll 