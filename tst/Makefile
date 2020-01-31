SPASM = spasm
CGEN ?= ../cgen-main

SPASM_FLAGS = -E -L

BENCHMARK = ./benchmark.sh
BENCHMARK_DIR = benchmark
BENCHMARK_CONF = $(BENCHMARK_DIR).conf
BENCHMARK_OUT = $(BENCHMARK_DIR).out

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

.PHONY: benchmark
benchmark:
	$(BENCHMARK) $(BENCHMARK_DIR) $(BENCHMARK_CONF) $(BENCHMARK_OUT) && column -t $(BENCHMARK_OUT)

