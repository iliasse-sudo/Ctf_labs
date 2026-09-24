FROM ubuntu:24.04

RUN apt-get update && apt-get install -y gcc make patchelf && rm -rf /var/lib/apt/lists/*

WORKDIR /challenge

# Extract libc and ld from the system
RUN cp /lib/x86_64-linux-gnu/libc.so.6 ./libc.so.6 && \
    cp /lib64/ld-linux-x86-64.so.2 ./ld-linux-x86-64.so.2

COPY main.c Makefile flag.txt run.sh ./
RUN make && chmod +x run.sh

EXPOSE 1342
ENTRYPOINT ["./run.sh"]