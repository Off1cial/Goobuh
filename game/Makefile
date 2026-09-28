.PHONY: build run clean


build:
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
	ninja -C build

run: build
	LSAN_OPTIONS="suppressions=$(CURDIR)/lsan.supp" ./build/game

clean:
	rm -rf build
