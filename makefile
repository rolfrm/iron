OPT = -g
SOURCES = $(wildcard *.c)
CC = gcc
TARGET = test
OBJECTS =$(SOURCES:.c=.o)
LDFLAGS=-ldl -L. -L../libconcurrency-read-only/  $(OPT) -Wextra  #setrlimit on linux 
LIBS= -ldl -lm -lpthread -lglfw -lGL -lpng
CFLAGS = -std=c11 -c $(OPT) -Wall -Wextra -Werror=implicit-function-declaration -Wformat=0  -g0 -O4 -D_GNU_SOURCE  -fdiagnostics-color
all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -ldl -o $@

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ -MMD -MF $@.depends
depend: h-depend
clean:
	rm $(OBJECTS) $(TARGET) *.o.depends
-include $(OBJECTS:.o=.o.depends)

