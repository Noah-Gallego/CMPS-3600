# CMPS 3600

Operating-systems coursework in C, Linux systems programming, concurrency, memory, scheduling, and security.

[![C](https://img.shields.io/badge/C-systems%20programming-A8B9CC?style=flat-square&logo=c&logoColor=black)](https://en.wikipedia.org/wiki/C_(programming_language))

## Course context

The repository preserves labs and semester-project work for CMPS 3600 at California State University, Bakersfield, Spring 2025. The original course notes identify Linux/Odin, GCC, GDB, Unix utilities, and Makefiles as the working environment.

## Contents

Numbered directories and lettered directories contain lab and project phases. Topics represented by the source include file I/O, processes, signals, interprocess communication, System V message queues and shared memory, semaphores, POSIX threads, mutexes, dining philosophers, pipes, memory mapping, page faults, and scheduling-related exercises.

- `1/`–`7/`, `9/`, `a/`–`e/` — lab and phase source files
- `Makefile` files — directory-specific build definitions
- `lab-start.sh` and `lab-fix.sh` — repository scripts

Compiled binaries and logs are also committed in several directories; they are preserved course artifacts, not required source dependencies.

## Usage

Build from an individual lab directory with that directory's Makefile. For example:

```bash
cd 1
make
```

The exact targets vary by directory. Review the local Makefile and source comments before running programs, especially exercises that create IPC objects or write files.

## Status and academic note

This is coursework and lab material, not a supported operating-systems library. Some directories contain platform-specific or historical binaries, so builds may require a compatible Linux toolchain.

## Attribution

Course and instructor details are retained from the repository's original documentation. No license is declared in this checkout.
