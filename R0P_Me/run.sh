#!/bin/bash

cd /home/ctf
exec socat TCP-LISTEN:1340,reuseaddr,fork \
    EXEC:"/home/ubuntu/r0p_me",stderr
