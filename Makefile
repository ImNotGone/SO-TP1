include Makefile.inc
SOURCES_ADT=$(wildcard ADTs/*.c)
OBJECTS_ADT=$(SOURCES_ADT:.c=.o)
all: $(BINARIES) $(OBJECTS_ADT)

debug: COMP_FLAGS+=-g
debug: LD_FLAGS+=-g
debug: all

$(OBJECTS_ADT): $(SOURCES_ADT)
	cd ADTs; make all

%.o: %.c
	$(COMP) $(COMP_FLAGS) -c $< -o $@

md5: md5.o $(OBJECTS_ADT)
	$(LD) $(LD_FLAGS) $(OBJECTS_ADT) $< -o $@

slave: slave.o $(OBJECTS_ADT)
	$(LD) $(LD_FLAGS) $(OBJECTS_ADT) $< -o $@

view: view.o $(OBJECTS_ADT)
	$(LD) $(LD_FLAGS) $(OBJECTS_ADT) $< -o $@

testwrite: testwrite.o
	$(LD) $(LD_FLAGS) $(OBJECTS_ADT) $< -o $@

testread: testread.o
	$(LD) $(LD_FLAGS) $(OBJECTS_ADT) $< -o $@

$(CREDENTIALS):
	    pvs-studio-analyzer credentials "PVS-Studio Free" "FREE-FREE-FREE-FREE"

pvs: $(CREDENTIALS)
	make clean
	pvs-studio-analyzer trace -- make
	pvs-studio-analyzer analyze
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log

clean:
	@rm -rf *.o
	@rm -rf $(BINARIES)
	@rm -rf $(MD5_OUTPUT)
	@rm -rf $(PVS_OUTPUT)
	@cd ADTs; make clean

.PHONY: all clean debug pvs
