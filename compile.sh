#!/bin/bash

echo "Compiling Catacombs for Unix systems through GCC"
if command -v gcc &> /dev/null; then
    gcc -o catacombs catacombs.c -lm
else
    echo "GCC does not exist on the current system. Exiting."
fi