.PHONY: build build-release run-release clean

build:
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
	cmake --build build

build-release:
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
	cmake --build build

run-release: build-release
ifeq ($(OS),Windows_NT)
	.\build\bin\engine.exe
else
	./build/bin/engine
endif

run: build
ifeq ($(OS),Windows_NT)
	.\build\bin\engine.exe
else
	./build/bin/engine
endif

clean:
	cmake --build build --target clean
