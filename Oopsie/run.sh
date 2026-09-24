#!/bin/bash
./oopsie
EXIT_CODE=$?

if [ $EXIT_CODE -eq 139 ]; then
    echo "Crash detected! Here is your flag:"
    cat flag.txt
else
    echo "Program exited normally (code: $EXIT_CODE). Try overflowing the buffer!"
fi