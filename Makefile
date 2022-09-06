COMP=gcc
COMP_FLAGS=-Wall -std=c99 -pedantic -lrt
BINARIES=md5 slave view
LD=gcc
LD_FLAGS=-Wall -std=c99 -pedantic -lrt

all: $(BINARIES)

debug: COMP_FLAGS+=-g
debug: LD_FLAGS+=-g
debug: all

%.o: %.c
	$(COMP) $(COMP_FLAGS) -c $< -o $@

md5: md5.o
	$(LD) $(LD_FLAGS) $< -o $@

slave: slave.o
	$(LD) $(LD_FLAGS) $< -o $@

view: view.o
	$(LD) $(LD_FLAGS) $< -o $@

clean:
	@rm -rf *.o
	@rm -rf $(BINARIES)

.PHONY: all clean debug
