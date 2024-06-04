#!/bin/bash

HOST1="localhost:8000"
HOST2="localhost:8080"
HOST_IPV6="::1"

# Function to test GET requests and compare responses
test_get_compare() {
    URL=$1
    HOST_REF=$2
    HOST_TEST=$3
    echo "Comparing GET $URL between $HOST_REF and $HOST_TEST"
    echo ""
    curl -s "http://$HOST_REF$URL" > response_ref.txt
    curl -s "http://$HOST_TEST$URL" > response_test.txt
    if ! diff response_ref.txt response_test.txt > /dev/null; then
        echo "!!! FAIL !!! GET $URL on $HOST_TEST differs from $HOST_REF"
        diff --color=auto response_ref.txt response_test.txt
    else
        echo "PASS: GET $URL on $HOST_TEST matches $HOST_REF"
    fi
    rm response_ref.txt response_test.txt
    echo ""
    echo ""
}

# Function to test POST requests and compare responses
test_post_compare() {
    URL=$1
    HOST_REF=$2
    HOST_TEST=$3
    DATA=$4
    echo "Comparing POST $URL between $HOST_REF and $HOST_TEST with data $DATA"
    echo ""
    curl -s -d $DATA -X POST "http://$HOST_REF$URL" > response_ref.txt
    curl -s -d $DATA -X POST "http://$HOST_TEST$URL" > response_test.txt
    if ! diff response_ref.txt response_test.txt > /dev/null; then
        echo "!!! FAIL !!! POST $URL on $HOST_TEST differs from $HOST_REF"
        diff --color=auto response_ref.txt response_test.txt
    else
        echo "PASS: POST $URL on $HOST_TEST matches $HOST_REF"
    fi
    rm response_ref.txt response_test.txt
    echo ""
    echo ""
}

echo ""
# Run comparisons
test_get_compare "/" $HOST1 $HOST2
test_post_compare "/submit" $HOST1 $HOST2 "name=example&value=test"

# Uncomment to test IPv6 comparisons
# test_get_compare "/" $HOST1 $HOST_IPV6
# test_post_compare "/submit" $HOST1 $HOST_IPV6 "name=example&value=test"

wait
