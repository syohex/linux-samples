CFLAGS=-Wall

TARGETS=run_after_exit

all: $(TARGETS)

run_after_exit: pidfd_open/run_after_exit.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: format clean

format:
	git ls-files | grep -E '\.(c|cpp|cc|h)$$' | xargs clang-format -i

clean:
	rm -f $(TARGETS)
