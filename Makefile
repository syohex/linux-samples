CFLAGS=-Wall -std=gnu99
CXXFLAGS=-Wall -std=gnu++11

TARGETS= \
	copy_dir \
	run_after_exit \
	getrandom_bytes

all: $(TARGETS)

copy_dir: copy_file_range/copy_dir.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

getrandom_bytes: getrandom/getrandom_bytes.c
	$(CC) $(CFLAGS) -o $@ $<

run_after_exit: pidfd_open/run_after_exit.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: format clean

format:
	git ls-files | grep -E '\.(c|cpp|cc|h)$$' | xargs clang-format -i

clean:
	rm -f $(TARGETS)
