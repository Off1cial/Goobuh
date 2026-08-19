.PHONY: build run clean

build:
	./build.sh

run: build
	LSAN_OPTIONS="suppressions=$(CURDIR)/lsan.supp" ./build/engine

clean:
	rm -rf build

debug:
	./build.sh Debug

release:
	./build.sh Release

debug-run:
	./build.sh Debug run

release-run:
	./build.sh Release run
