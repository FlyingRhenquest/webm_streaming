#!/bin/bash

ffserver -f ffserver.conf &
./startstreams.sh &
./server &
tinyproxy -c tinyproxy.conf &
python3 -m http.server 8000 --bind 127.0.0.1 &
