cd /home/ctf
exec socat -d -d \
    TCP-LISTEN:1338,reuseaddr,fork, \
    EXEC:"/home/ctf/detector.sh",stderr
