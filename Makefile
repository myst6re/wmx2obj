CC = gcc
OBJS = $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c))
TARGET = bin/wmx2obj
CFLAGS = \
	-Wall -Wextra -Wpedantic -Wformat=2 -Wno-unused-parameter -Wshadow -Wwrite-strings \
	-Wstrict-prototypes -Wold-style-definition -Wredundant-decls -Wnested-externs \
	-Wmissing-include-dirs -Wjump-misses-init -Wlogical-op -std=c11

.PHONY : all clean

all : $(OBJS)
	mkdir -p $(dir $(TARGET))
	$(CC) $(OBJS) -o $(TARGET)

clean :
	$(RM) $(OBJS) $(TARGET)

obj/%.o : src/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)
