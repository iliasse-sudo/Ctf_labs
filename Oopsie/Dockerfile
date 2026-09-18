FROM gcc:13 AS builder

WORKDIR /build

COPY main.c Makefile ./

RUN make

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    socat \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -s /bin/bash ctf

WORKDIR /home/ctf

COPY --from=builder /build/Oopsie ./
COPY flag.txt ./
COPY entrypoint.sh ./

RUN chown -R ctf:ctf /home/ctf && \
    chmod +x entrypoint.sh && \
    chmod 444 flag.txt

USER ctf

EXPOSE 1337

ENTRYPOINT ["./entrypoint.sh"]