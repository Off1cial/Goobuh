.PHONY: build run clean


build:
	./build.sh

run: build
	LSAN_OPTIONS="suppressions=$(CURDIR)/lsan.supp" ./build/engine

clean:
	rm -rf build
