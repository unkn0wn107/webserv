#!/bin/bash

HOST="localhost"
PORT="8080"
HOST_IPV6="::1"

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

test_get_ipv6() {
    URL=$1
    EXPECTED_STATUS=$2
    echo "Testing GET $URL on IPv6 expecting $EXPECTED_STATUS"
    RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -g "http://[$HOST_IPV6]:$PORT$URL")
    if [ "$RESPONSE" == "$EXPECTED_STATUS" ]; then
        echo "PASS: $URL on IPv6"
    else
        echo "FAIL: $URL on IPv6 Expected $EXPECTED_STATUS but got $RESPONSE"
    fi
}

test_post_ipv6() {
    URL=$1
    EXPECTED_STATUS=$2
    DATA=$3
    echo "Testing POST $URL on IPv6 with data $DATA expecting $EXPECTED_STATUS"
    RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -d $DATA -X POST -g "http://[$HOST_IPV6]:$PORT$URL")
    if [ "$RESPONSE" == "$EXPECTED_STATUS" ]; then
        echo "PASS: $URL on IPv6"
    else
        echo "FAIL: $URL on IPv6 Expected $EXPECTED_STATUS but got $RESPONSE"
    fi
}

test_get "/" "200" &
test_get "/nonexistent" "404" &
# test_post "/submit" "200" "name=example&value=test" &

test_get_ipv6 "/" "200" &
test_get_ipv6 "/nonexistent" "404" &
# test_post_ipv6 "/submit" "200" "name=example&value=test" &

wait
