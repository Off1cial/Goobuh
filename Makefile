.PHONY: build build-release run-release clean

build:
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=gcc
	cmake --build build

build-release:
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=gcc
	cmake --build build

run-release: build-release
ifeq ($(OS),Windows_NT)
	.\build\bin\CEngine.exe
else
	./build/bin/CEngine
endif

run: build
ifeq ($(OS),Windows_NT)
	.\build\bin\CEngine.exe
else
	./build/bin/CEngine
endif

clean:
	cmake --build build --target clean
