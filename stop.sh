#!/bin/bash

docker-compose down
docker image rm ctf_builder ctf_labs-oopsie ctf_labs-returnal ctf_labs-r0p_me
rm -rf player_challenges
