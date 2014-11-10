CFLAGS=-mno-cygwin -Os
LDFLAGS=-mno-cygwin -s -Os

CC32 := i386-mingw32-gcc
CC64 := x86_64-mingw32-gcc

all: winexesvc32_exe.c winexesvc64_exe.c

winexesvc32.exe: winexesvc32.o service32.o
	$(CC32) $(LDFLAGS) -o $@ $^

winexesvc64.exe: winexesvc64.o service64.o
	$(CC64) $(LDFLAGS) -o $@ $^

%32.o: %.c
	$(CC32) -c $(CPPFLAGS) $(CFLAGS) -o $@ $^

%64.o: %.c
	$(CC64) -c $(CPPFLAGS) $(CFLAGS) -o $@ $^

winexesvc32_exe.c: winexesvc32.exe bin2c.exe
	./bin2c.exe winexesvc32_exe winexesvc32.exe > $@

winexesvc64_exe.c: winexesvc64.exe bin2c.exe
	./bin2c.exe winexesvc64_exe winexesvc64.exe > $@

bin2c.exe: bin2c.c
	gcc -s -o $@ $^

clean:
	-@rm *.exe *.o *_exe.c
