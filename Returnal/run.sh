#!/bin/bash

cd /home/ctf
exec socat \
    TCP-LISTEN:1339,reuseaddr,fork \
    EXEC:"/home/ubuntu/returnal",pty,stderr,setsid,sigint,sane
#    EXEC:"/home/ubuntu/returnal",stderr \
