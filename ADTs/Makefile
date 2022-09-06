include ../Makefile.inc
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)

all: $(OBJECTS)

%.o: %.c
	$(COMP) $(COMP_FLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJECTS)

.PHONY: all clean
