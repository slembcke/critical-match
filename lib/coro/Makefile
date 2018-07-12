.PHONY: clean test

clean:
	rm -rf *.o coro_test link.log

test: coro_test
	sim65 $<

# Cancel built in rules.
%.o: %.c

%.s: %.c
	cc65 -g -Oirs $< --add-source $(INCLUDE) -o $@

%.o: %.s
	ca65 -g $< -o $@

coro_test: coro_test.o coro.o
	ld65 -C ld65.cfg -vm -m link.log $^ sim6502.lib -o $@