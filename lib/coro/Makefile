.PHONY: test
test: coro_test
	sim65 $<

coro_test: coro_test.c coro.s
	cl65 -t sim6502 $^ -o $@