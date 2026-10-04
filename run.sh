#!/bin/bash

docker build -t ctf_builder -f Builder/Dockerfile .
docker run -d --name builder ctf_builder
mkdir -p player_challenges
docker cp builder:/Oopsie/Oopsie.tar.gz player_challenges/
docker cp builder:/Returnal/Returnal.tar.gz player_challenges/
docker rm builder
docker-compose up --build -d oopsie returnal
