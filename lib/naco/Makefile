.PHONY: clean test

clean:
	rm -rf *.o naco_test link.log

test: naco_test
	sim65 -cv $<

# Cancel built in rules.
%.o: %.c

%.s: %.c
	cc65 -g -Oirs $< --add-source $(INCLUDE) -o $@

%.o: %.s
	ca65 -g $< -o $@

naco_test: naco_test.o naco.o
	ld65 -C ld65.cfg -vm -m link.log $^ sim6502.lib -o $@