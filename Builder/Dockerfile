FROM gcc:13 AS builder


WORKDIR /Oopsie
COPY Oopsie/main.c Oopsie/challenge.txt Oopsie/Makefile ./
RUN make server && make player

WORKDIR /Returnal
COPY Returnal/main.c Returnal/challenge.txt Returnal/Makefile ./
RUN make server && make player

WORKDIR /R0P_Me
COPY R0P_Me/main.c R0P_Me/challenge.txt R0P_Me/Makefile ./
RUN make server && make player
