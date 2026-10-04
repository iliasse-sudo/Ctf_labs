#!/bin/bash

docker-compose down
docker image rm ctf_builder ctf_labs-oopsie ctf_labs-returnal
rm -rf player_challenges
