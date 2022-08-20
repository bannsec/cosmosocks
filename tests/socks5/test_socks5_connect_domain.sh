#!/bin/bash

# Print script file name
echo "Testing: $0 $1 $2"

# Start the socks server
$1 -p $2 &
SOCKS_PID=$!
sleep 0.5

STATUS=`timeout 10s curl -s -x socks5h://localhost:$2 -w "%{http_code}" -o /dev/null https://google.com`

if [ $STATUS -ne 301 ]; then
    echo "Error: status code is not 301"
    kill -9 $SOCKS_PID
    exit 1
fi

kill -9 $SOCKS_PID
exit 0
