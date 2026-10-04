FROM ctf_builder AS builder


FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    socat \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -s /bin/bash ctf

WORKDIR /home/ctf

COPY flag.txt ./

WORKDIR /home/ubuntu

COPY --from=builder /Returnal/returnal ./

COPY run.sh ./

RUN chown -R ctf:ctf /home/ctf && \
    chown -R root:root /home/ubuntu && \
    chmod 755 /home/ubuntu/returnal /home/ubuntu/run.sh

USER ctf

EXPOSE 1339

ENTRYPOINT ["/home/ubuntu/run.sh"]
