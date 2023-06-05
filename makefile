OPT = -g0 -O4
SOURCES = $(wildcard *.c)
SOURCES := $(filter-out duck_img.png.c texture.shader.c image.c testmain.c coroutines2.c  ,$(SOURCES))
CC = gcc
TARGET = libiron.a
OBJECTS =$(SOURCES:.c=.o)
LDFLAGS= -L. $(OPT) -Wextra -shared # -fPIC #setrlimit on linux 
LIBS=  -lm -lpthread -lglfw -lGL -lX11 -lopenal -lpng #-ldl
CFLAGS_BASIC = -std=c11 -c $(OPT) -Wall -Wextra  -Werror -Wno-deprecated -Wsign-compare -Wstrict-prototypes
CFLAGS = $(CFLAGS_BASIC) #-fPIC

all: $(TARGET) libiron.a
#libiron.so: $(OBJECTS)
#	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

libiron.a: $(OBJECTS)
	ar rcs libiron.a $(OBJECTS) 
libiron.bc: $(OBJECTS)
	emcc $(OBJECTS) -r -s WASM=1 -s USE_GLFW=3 -o libiron.bc
cutils.o: cutils.c
	$(CC) -c  $< -o $@ 

libbf.o: libbf.c 
	$(CC) -c  $< -o $@ 

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ -MMD -MF $@.depends
gl.o: texture.shader.c
test: duck_img.png.c

texture.shader.c: texture.shader.fs texture.shader.vs texture.3d.shader.vs texture.3d.color.shader.vs texture.depth.shader.fs voxel_depth.fs voxel_depth.vs voxel_depth2.fs
	xxd -i texture.shader.vs > texture.shader.c
	xxd -i texture.3d.shader.vs >> texture.shader.c
	xxd -i texture.3d.color.shader.vs >> texture.shader.c
	xxd -i texture.shader.fs >> texture.shader.c
	xxd -i texture.depth.shader.fs >> texture.shader.c
	xxd -i voxel_depth.fs >> texture.shader.c
	xxd -i voxel_depth.vs >> texture.shader.c
	xxd -i voxel_depth2.fs >> texture.shader.c

duck_img.png.c: duck.png
	xxd -i duck.png > duck_img.png.c

wasm: CC = emcc
wasm: CFLAGS = -std=c11 -c $(OPT) -Wall -Wextra -Werror=implicit-function-declaration -Wformat=0 -D_GNU_SOURCE  -fdiagnostics-color -D_EMCC_
wasm: LDFLAGS= -r -s WASM=1 -s USE_GLFW=3
wasm: LIBS =  -ldl -lm -lpthread -lglfw -lGL -lopenal
wasm: libiron.a
wasm: libiron.bc

depend: h-depend
clean:
	rm $(OBJECTS) $(TARGET) *.o.depends *.bc *.bc.depends test libiron.a

-include $(OBJECTS:.o=.o.depends)

test: $(OBJECTS) testmain.c
	$(CC) testmain.c $(OBJECTS) -L/usr/lib/nvidia-384/ $(LIBS) $(OPT) -o $@


install: $(TARGET)
	mkdir -v /usr/include/iron |true
	cp -v ./*.h /usr/include/iron
	cp -v ./libiron.so /usr/lib/
