#!/bin/bash

docker run -it --rm -v /mnt/d:/mnt/d -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix ubuntu24.dyssolopen
