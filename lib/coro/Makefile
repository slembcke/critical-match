.PHONY: test
test: coro_test
	sim65 $<

%.s: %.c
	cc65 $(CFLAGS) $< --add-source $(INCLUDE) -o $@

coro_test: coro_test.s coro.s
	cl65 -t sim6502 $^ -o $@