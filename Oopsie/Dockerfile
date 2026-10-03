FROM gcc:13 AS builder

WORKDIR /build

COPY main.c Makefile ./

RUN make server

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    socat \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -s /bin/bash ctf

WORKDIR /home/ctf

COPY --from=builder /build/oopsie ./
COPY flag.txt ./
COPY run.sh ./
COPY detector.sh ./

RUN chown -R ctf:ctf /home/ctf && \
    chmod +x run.sh detector.sh && \
    chmod 444 flag.txt

USER ctf

EXPOSE 1338

ENTRYPOINT ["./run.sh"]
