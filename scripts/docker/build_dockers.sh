#!/bin/bash

docker build -f Dockerfile.ubuntu20.dyssolopen -t ubuntu20.dyssolopen .
docker build -f Dockerfile.ubuntu22.dyssolopen -t ubuntu22.dyssolopen .
docker build -f Dockerfile.ubuntu24.dyssolopen -t ubuntu24.dyssolopen .
docker build -f Dockerfile.debian11.dyssolopen -t debian11.dyssolopen .
docker build -f Dockerfile.debian12.dyssolopen -t debian12.dyssolopen .
docker build -f Dockerfile.debiansid.dyssolopen -t debiansid.dyssolopen .
