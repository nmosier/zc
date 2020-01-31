SPASM = spasm
CGEN ?= ../cgen-main

SPASM_FLAGS = -E -L

%.8xp: %.z80
	$(SPASM) $(SPASM_FLAGS) $^ $@
%.bin: %.z80
	$(SPASM) $(SPASM_FLAGS) $^ $@

bare-%.z80: %.c
	cp bare.inc $@
	$(CGEN) $(CFLAGS) < $^ >> $@

%.z80: %.c
	cp preamble.z80 $@
	$(CGEN) $(CFLAGS) < $^ >> $@
