# Project README

## Overview
This project is a simple text-based IDE (Integrated Development Environment) written in C. It features a graphical user interface with functionalities to edit and run code, primarily targeting Linux, Windows, Wine, and WebAssembly environments.

## Features
- **Graphical User Interface**: Provides an editor for writing C code.
- **Code Execution**: Allows running the compiled code within the IDE.
- **Platform Compatibility**:
  - Linux
  - Windows
  - Cross-compilation for Windows on Linux using Wine
  - Web-based execution (WebAssembly)

## Project Structure
```
<Project>/
├── build/              
├── bin/                
├── libs/               
├── lib/                
├── code/               
├── data/               
├── assets/             
├── src/                # Source files
│   ├── Main.c          # Entry point
│   └── Saved.h         # Standalone header file
├── Makefile.linux      # Linux Build configuration
├── Makefile.windows    # Windows Build configuration
├── Makefile.wine       # Wine Build configuration
├── Makefile.web        # Emscripten Build configuration
└── README.md           
└── LICENSE             
└── .gitignore
```

## Prerequisites
- **C/C++ Compiler and Debugger**: GCC, Clang (for Linux), Visual Studio for Windows.
- **Make utility**: To build the project using Makefiles.
- **Standard development tools**: For compiling C code and managing dependencies.
- **Libraries**:
  - X11 (Linux)
  - user32, gdi32, winmm (Windows)
  - WINE (for cross-compilation to Windows on Linux)
  - emcc (Emscripten for WebAssembly)

## Build & Run
### Build Process
Navigate to the project directory and run:
```bash
make -f Makefile.(os) all
```
Replace `(os)` with `linux`, `windows`, `wine`, or `web` depending on your target platform.

#### Clean Rebuild
For a clean rebuild, first remove existing build artifacts and then build again:
```bash
make -f Makefile.(os) clean
make -f Makefile.(os) all
```

### Build Options
- **Build output**: `make -f Makefile.(os) all`
- **Build + exe output**: `make -f Makefile.(os) do`
- **Remove build artifacts**: `make -f Makefile.(os) clean`

### Execute
To execute the built application:
```bash
make -f Makefile.(os) exe
```

This README provides a comprehensive guide on how to set up and run the project in different environments, ensuring that all dependencies are accounted for.