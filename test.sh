#!/bin/bash

HOST="localhost"
PORT="8080"

test_get() {
    URL=$1
    EXPECTED_STATUS=$2
    echo "Testing GET $URL expecting $EXPECTED_STATUS"
    RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" http://$HOST:$PORT$URL)
    if [ "$RESPONSE" == "$EXPECTED_STATUS" ]; then
        echo "PASS: $URL"
    else
        echo "FAIL: $URL Expected $EXPECTED_STATUS but got $RESPONSE"
    fi
}

test_post() {
    URL=$1
    EXPECTED_STATUS=$2
    DATA=$3
    echo "Testing POST $URL with data $DATA expecting $EXPECTED_STATUS"
    RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -d $DATA -X POST http://$HOST:$PORT$URL)
    if [ "$RESPONSE" == "$EXPECTED_STATUS" ]; then
        echo "PASS: $URL"
    else
        echo "FAIL: $URL Expected $EXPECTED_STATUS but got $RESPONSE"
    fi
}

test_get "/" "200" &
test_get "/nonexistent" "404" &
# test_post "/submit" "200" "name=example&value=test" &

wait
