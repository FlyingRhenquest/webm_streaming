#!/bin/bash

ffmpeg -f lavfi -i "testsrc=duration=-1:size=1600x1200:rate=30" \
       -f lavfi -i sine \
       -c:v libvpx -threads 4 -speed 6 -pix_fmt yuv420p -b:v 512k -r 30 \
       -c:a libopus -threads 4 -b:a 256k -ac 2 -ar 48000 \
       -f ffm http://127.0.0.1:8090/test_video.ffm
