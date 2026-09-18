#!/bin/bash

cd /home/ctf
exec socat \
    TCP-LISTEN:1337,reuseaddr,fork \
    EXEC:"/home/ctf/Oopsie",stderr