.PHONY: build run clean

build:
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
	cmake --build build

run: build
	.\build\engine.exe

clean:
	cmake --build build --target clean
