FROM ubuntu:22.04

RUN apt-get update && apt-get install -y gcc make && rm -rf /var/lib/apt/lists/*

WORKDIR /challenge
COPY main.c Makefile flag.txt run.sh ./
RUN make && chmod +x run.sh

EXPOSE 1339
ENTRYPOINT ["./run.sh"]