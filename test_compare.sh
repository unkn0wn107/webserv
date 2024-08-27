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
    curl -i -s "http://$HOST_REF$URL" > response_ref.txt
    curl -i -s "http://$HOST_TEST$URL" > response_test.txt
    diff --color=auto response_ref.txt response_test.txt
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
    curl -i -s -d $DATA -X POST "http://$HOST_REF$URL" > response_ref.txt
    curl -i -s -d $DATA -X POST "http://$HOST_TEST$URL" > response_test.txt
    diff --color=auto response_ref.txt response_test.txt
    rm response_ref.txt response_test.txt
    echo ""
    echo ""
}

echo ""
# Run comparisons
test_get_compare "/" $HOST1 $HOST2
test_get_compare "/blabla/" $HOST1 $HOST2
test_get_compare "/upload/" $HOST1 $HOST2
test_get_compare "/redirect/" $HOST1 $HOST2
test_post_compare "/redirect/" $HOST1 $HOST2
test_post_compare "/submit/" $HOST1 $HOST2
test_post_compare "/submit/" $HOST1 $HOST2 "name=example&value=test"

# Uncomment to test IPv6 comparisons
# test_get_compare "/" $HOST1 $HOST_IPV6
# test_post_compare "/submit" $HOST1 $HOST_IPV6 "name=example&value=test"

wait
