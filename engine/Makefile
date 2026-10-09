.PHONY: build build-release run-release clean

build:
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
	cmake --build build

build-release:
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
	cmake --build build

run-release: build-release
ifeq ($(OS),Windows_NT)
	.\build\engine.exe
else
	./build/engine
endif

run: build
ifeq ($(OS),Windows_NT)
	.\build\engine.exe
else
	./build/engine
endif

clean:
	cmake --build build --target clean
