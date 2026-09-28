#!/bin/bash

cd /home/ctf
exec socat \
    TCP-LISTEN:1338,reuseaddr,fork \
    EXEC:"/home/ctf/detector.sh",stderr
