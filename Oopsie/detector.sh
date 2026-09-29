#!/bin/bash
{ ./oopsie 2>/dev/null; } 2>/dev/null
EXIT_CODE=$?

if [ "$EXIT_CODE" -eq 139 ]; then
    echo "Buffer overflow detected! Here is your flag:"
    cat flag.txt
else
    echo "No flag for you yet!"
fi
echo -e "\nCRTL+D to exit"