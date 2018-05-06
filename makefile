OPT = -g3 -Og
SOURCES = $(wildcard *.c)
SOURCES := $(filter-out gl.c testmain.c, $(SOURCES))
CC = gcc
TARGET = libiron.so
OBJECTS =$(SOURCES:.c=.o)
LDFLAGS=-ldl -L. -L../libconcurrency-read-only/  $(OPT) -Wextra -shared -L/usr/lib/nvidia-384/ -fPIC #setrlimit on linux 
LIBS= -ldl -lm -lpthread  -lGL -lpng 
CFLAGS = -std=c11 -c $(OPT) -Wall -Wextra -Werror=implicit-function-declaration -Wformat=0  -g0 -O4 -msse4.1 -D_GNU_SOURCE  -fdiagnostics-color -shared -fPIC
all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -ldl -o $@

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ -MMD -MF $@.depends
depend: h-depend
clean:
	rm $(OBJECTS) $(TARGET) *.o.depends
-include $(OBJECTS:.o=.o.depends)

test: $(OBJECTS) testmain.c
	$(CC) testmain.c $(OBJECTS) -L/usr/lib/nvidia-384/ $(LIBS) $(OPT) -o $@


install: $(TARGET)
	mkdir -v /usr/include/iron |true
	cp -v ./*.h /usr/include/iron
	cp -v ./libiron.so /usr/lib/
