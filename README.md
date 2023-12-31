# Tar Fuzzer

## Overview

The Tar Fuzzer is a C program designed to test the robustness and security of TAR file extraction utilities. It generates various malformed TAR files to check how the TAR extractor handles different edge cases and incorrect data formats.

## Repository Structure

- `fuzzer.c`: The main C file containing the fuzzer logic.

## Getting Started

### Prerequisites

- A C compiler (e.g., GCC)
- Standard C libraries
- TAR extraction utility for testing

### Compilation

To compile the Tar Fuzzer, use the Makefile:


```bash
make
```
### Usage

Run the Tar Fuzzer by providing the path to the TAR extractor as an argument:


```bash
./fuzzer [path_to_tar_extractor]
```

The fuzzer will generate various TAR files and use the specified extractor to test them. It's important to monitor the extractor's behavior for any crashes, unexpected behavior, or security issues.

### Cleaning Up

After running the fuzzer, clean up the generated test files with:


```bash
make clean
```