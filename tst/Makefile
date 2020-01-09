SPASM = spasm
CGEN = ../cgen-main

SPASM_FLAGS = -E -L

%.8xp: %.z80
	$(SPASM) $(SPASM_FLAGS) $^ $@

%.z80: %.c
	cp preamble.z80 $@
	$(CGEN) < $^ >> $@
