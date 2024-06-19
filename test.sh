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

test_cgi_hello() {
    URL="/cgi/hello.py"
    EXPECTED_STATUS="200"
    EXPECTED_CONTENT_TYPE="text/html"
    echo "Testing GET $URL expecting $EXPECTED_STATUS and Content-Type $EXPECTED_CONTENT_TYPE"
    RESPONSE=$(curl -i -s -o response.txt -w "%{http_code}" http://$HOST:$PORT$URL)
    if [ -f response.txt ]; then
        CONTENT_TYPE=$(curl -i -s http://$HOST:$PORT$URL | grep 'Content-Type' | cut -d ':' -f2- | tr -d ' \r\n\t')
        if [ "$RESPONSE" == "$EXPECTED_STATUS" ] && [[ "$CONTENT_TYPE" == *"$EXPECTED_CONTENT_TYPE"* ]]; then
            echo "PASS: $URL"
        else
            echo "FAIL: $URL Expected $EXPECTED_STATUS and $EXPECTED_CONTENT_TYPE but got $RESPONSE and $CONTENT_TYPE"
            cat response.txt
        fi
        rm response.txt
    else
        echo "FAIL: $URL - No response received"
    fi
}
return_failure_if_test_fails() {
    if [ "$1" != "0" ]; then
        echo "One or more tests failed."
        exit 1
    fi
}

# Run tests and capture their exit status
test_get "/" "200"
test_get "/nonexistent" "404"
test_get "/cgi/hello.py" "200"
test_get "/cgi/hello.php" "200"
test_get "/cgi/off/disabled.py" "403"
# test_post "/submit" "200" "name=example&value=test"

test_get_ipv6 "/" "200"
test_get_ipv6 "/nonexistent" "404"
# test_post_ipv6 "/submit" "200" "name=example&value=test"

test_cgi_hello

# Check if any test failed
return_failure_if_test_fails $?
