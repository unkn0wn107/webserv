#!/bin/bash

HOST1="localhost:8000"
HOST2="localhost:8080"
HOST_IPV6="::1"

test_get() {
    URL=$1
    HOST=$2
    EXPECTED_STATUS=$3
    echo "Testing GET $URL on $HOST expecting $EXPECTED_STATUS"
    RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" "http://$HOST$URL")
    if [ "$RESPONSE" != "$EXPECTED_STATUS" ]; then
        echo "FAIL: $URL on $HOST Expected $EXPECTED_STATUS but got $RESPONSE"
    fi
}

test_post() {
    URL=$1
    HOST=$2
    EXPECTED_STATUS=$3
    DATA=$4
    echo "Testing POST $URL on $HOST with data $DATA expecting $EXPECTED_STATUS"
    RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -d $DATA -X POST "http://$HOST$URL")
    if [ "$RESPONSE" != "$EXPECTED_STATUS" ]; then
        echo "FAIL: $URL on $HOST Expected $EXPECTED_STATUS but got $RESPONSE"
    fi
}

test_get_ipv6() {
    URL=$1
    HOST=$2
    EXPECTED_STATUS=$3
    echo "Testing GET $URL on $HOST (IPv6) expecting $EXPECTED_STATUS"
    RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -g "http://[$HOST]$URL")
    if [ "$RESPONSE" != "$EXPECTED_STATUS" ]; then
        echo "FAIL: $URL on $HOST (IPv6) Expected $EXPECTED_STATUS but got $RESPONSE"
    fi
}

test_post_ipv6() {
    URL=$1
    HOST=$2
    EXPECTED_STATUS=$3
    DATA=$4
    echo "Testing POST $URL on $HOST (IPv6) with data $DATA expecting $EXPECTED_STATUS"
    RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -d $DATA -X POST -g "http://[$HOST]$URL")
    if [ "$RESPONSE" != "$EXPECTED_STATUS" ]; then
        echo "FAIL: $URL on $HOST (IPv6) Expected $EXPECTED_STATUS but got $RESPONSE"
    fi
}

REF_STATUS_GET=$(test_get "/" $HOST1 "200" &> /dev/null; echo $?)
REF_STATUS_POST=$(test_post "/submit" $HOST1 "200" "name=example&value=test" &> /dev/null; echo $?)

test_get "/" $HOST2 "200"
if [ $? != $REF_STATUS_GET ]; then
    echo "FAIL: / on $HOST2 differs from $HOST1"
fi

test_post "/submit" $HOST2 "200" "name=example&value=test"
if [ $? != $REF_STATUS_POST ]; then
    echo "FAIL: /submit on $HOST2 differs from $HOST1"
fi

# test_get_ipv6 "/" $HOST_IPV6 "200"
# test_get_ipv6 "/nonexistent" $HOST_IPV6 "404"
# test_post_ipv6 "/submit" $HOST_IPV6 "200" "name=example&value=test"

wait
