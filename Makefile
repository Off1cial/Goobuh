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


server:
	./build.sh
	ninja -C build server

server-run:
	./build.sh
	ninja -C build server
	LSAN_OPTIONS="suppressions=$(CURDIR)/lsan.supp" ./build/server

server-debug:
	./build.sh Debug
	ninja -C build server

server-release:
	./build.sh Release
	ninja -C build server

server-debug-run:
	./build.sh Debug
	ninja -C build server
	LSAN_OPTIONS="suppressions=$(CURDIR)/lsan.supp" ./build/server

server-release-run:
	./build.sh Release
	ninja -C build server
	LSAN_OPTIONS="suppressions=$(CURDIR)/lsan.supp" ./build/server