FROM ctf_builder AS builder


FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    socat \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -s /bin/bash ctf

WORKDIR /home/ctf

COPY flag.txt ./

RUN echo "PS1='\[\033[1;32m\]ctf@Returnal\[\033[0m\]:\[\033[1;34m\]\w\[\033[0m\]\$ '" > /home/ctf/.bashrc

WORKDIR /home/ubuntu

COPY --from=builder /Returnal/returnal ./

COPY run.sh ./

RUN chown -R ctf:ctf /home/ctf && \
    chown -R root:root /home/ubuntu && \
    chmod 755 /home/ubuntu/returnal /home/ubuntu/run.sh

USER ctf

EXPOSE 1339

ENTRYPOINT ["/home/ubuntu/run.sh"]
