FROM gcc:13 AS builder


WORKDIR /Oopsie
COPY Oopsie/main.c Oopsie/challenge.txt Oopsie/Makefile ./
RUN make server && make player

WORKDIR /Returnal
COPY Returnal/main.c Returnal/challenge.txt Returnal/Makefile ./
RUN make server && make player

# WORKDIR /R0P_Me
# COPY R0P_Me/main.c R0P_Me/challenge.txt R0P_Me/Makefile ./
# RUN make server && make player

# WORKDIR /Blind_Spot
# COPY Blind_Spot/main.c Blind_Spot/challenge.txt Blind_Spot/Makefile ./
# RUN make server && make player

# WORKDIR /House_of_Calls
# COPY House_of_Calls/main.c House_of_Calls/challenge.txt House_of_Calls/Makefile ./
# RUN make server && make player

# WORKDIR /Final_Payload
# COPY Final_Payload/main.c Final_Payload/challenge.txt Final_Payload/Makefile ./
# RUN make server && make player






# WORKDIR /alive

# COPY Builder/run.sh ./

# RUN chmod +x run.sh

# ENTRYPOINT ["/alive/run.sh"]