# Agar.io

Clone of [agar.io](agar.io) game made in C. [SDL2](https://www.libsdl.org) is used for user interface.

## Requirements

- Unix-based operating system - project uses `pthreads.h` and `socket.h`
- [SDL2](https://www.libsdl.org)
- [CMake](https://cmake.org) (3.10+)


## Installation

- Create `build` directory
- Inside `build` directory run `cmake ..` to generate build files
- Run `make` to build both client and server executable
- Firstly, run server using `./server`
- Then use `./client` to run client instance
