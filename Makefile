CC=gcc
CFLAGS=-I/usr/local/include
LDFLAGS=-Wl,-rpath,. -lm -lSDL2 -lSDL2_image -lSDL2_mixer
RESOURCE_FILES = $(wildcard resource_files/*)

dist: sexy
	./make_unix_dist.py sexy sexy_lin64
	rm -f sexy
	tar -czvf sexy_lin64.tgz sexy_lin64

sexy: resources.o res.o framerate.o sexy.c
	$(CC) $(CFLAGS) -o sexy sexy.c SDL2_framerate.o res.o resources.o $(LDFLAGS)

framerate.o: SDL2_framerate.c SDL2_framerate.h 
	$(CC) $(CFLAGS) -c SDL2_framerate.c

res.o: resources.o
	$(CC) $(CFLAGS) -c res.c

resources.o: $(RESOURCE_FILES)
	./pack_resources.py resource_files
	$(CC) $(CFLAGS) -c resources.c

clean:
	rm -f *.o
	rm -f *~
	rm -f \#*

distclean: clean
	rm -f resources.h
	rm -f resources.c
	rm -f sexy
	rm -f sexy_lin64.tgz
