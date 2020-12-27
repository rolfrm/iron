OPT = -g3 -O0
SOURCES = $(wildcard *.c)
SOURCES := $(filter-out duck_img.png.c texture.shader.c image.c testmain.c coroutines2.c , $(SOURCES))
CC = gcc
TARGET = libiron.so
OBJECTS =$(SOURCES:.c=.o)
LDFLAGS=-ldl -L. -L../libconcurrency-read-only/  $(OPT) -Wextra -shared -L/usr/lib/nvidia-384/ -fPIC #setrlimit on linux 
LIBS= -ldl -lm -lpthread -lGL -lpng -lglfw -lX11 
CFLAGS_BASIC = -std=c11 -c $(OPT) -Wall -Wextra -Werror=implicit-function-declaration -Wformat=0  -msse4.1 -D_GNU_SOURCE  -fdiagnostics-color -Werror
CFLAGS = $(CFLAGS_BASIC) -shared -fPIC

all: test $(TARGET)
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -ldl -o $@
	ar rcs libiron.a $(OBJECTS)

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ -MMD -MF $@.depends
gl.o: texture.shader.c
test: duck_img.png.c
texture.shader.c: texture.shader.fs texture.shader.vs texture.3d.shader.vs
	xxd -i texture.shader.vs > texture.shader.c
	xxd -i texture.3d.shader.vs >> texture.shader.c
	xxd -i texture.shader.fs >> texture.shader.c

duck_img.png.c: duck.png
	xxd -i duck.png > duck_img.png.c

depend: h-depend
clean:
	rm $(OBJECTS) $(TARGET) *.o.depends test libiron.a

-include $(OBJECTS:.o=.o.depends)

test: $(OBJECTS) testmain.c
	$(CC) testmain.c $(OBJECTS) -L/usr/lib/nvidia-384/ $(LIBS) $(OPT) -o $@


install: $(TARGET)
	mkdir -v /usr/include/iron |true
	cp -v ./*.h /usr/include/iron
	cp -v ./libiron.so /usr/lib/
