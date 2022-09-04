
COMP=gcc
COMP_FLAGS=-Wall -std=c99 -pedantic
BINARIES=md5 slave vista
LD=gcc
LDFLAGS=$(COMP_FLAGS)

all: $(BINARIES)

%.o: %.c
	$(COMP) $(COMP_FLAGS) -c $< -o $@

md5: md5.o
	$(LD) $(LDFLAGS) $< -o $@

slave: slave.o
	$(LD) $(LDFLAGS) $< -o $@

vista: vista.o
	$(LD) $(LDFLAGS) $< -o $@

clean:
	rm -rf *.o
	rm -rf $(BINARIES)

.PHONY: all clean
