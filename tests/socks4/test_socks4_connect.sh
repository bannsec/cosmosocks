#!/bin/bash

# Print script file name
echo "Testing: $0 $1 $2"

# Start the socks server
$1 -p $2 &
SOCKS_PID=$!
sleep 0.5

# curl to https://8.8.8.8, checking status code response is 302. If not, exit with error.
STATUS=`timeout 10s curl -s -x socks4://localhost:$2 -w "%{http_code}" -o /dev/null https://8.8.8.8`

if [ $STATUS -ne 302 ]; then
    echo "Error: status code is not 302"
    kill -9 $SOCKS_PID
    exit 1
fi

kill -9 $SOCKS_PID
exit 0
