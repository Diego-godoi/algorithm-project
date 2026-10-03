CC      := cc
CFLAGS  := -Wall -Wextra -Wpedantic -g -std=c17 -D_POSIX_C_SOURCE=200809L
LDLIBS  := -lm
TARGET  := algorithm-project
SRCDIR  := app
SOURCES := $(shell find $(SRCDIR) -type f -name '*.c')

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) $(LDLIBS) -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: run clean
