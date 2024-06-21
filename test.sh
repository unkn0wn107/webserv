#!/bin/bash

HOST="localhost"
PORT="8080"
HOST_IPV6="::1"
TEST_FAILED=0

test_get() {
    URL=$1
    EXPECTED_STATUS=$2
    RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" http://$HOST:$PORT$URL)
    if [ "$RESPONSE" == "$EXPECTED_STATUS" ]; then
        echo "OK: GET $URL"
    else
        echo "!!!KO!!!: GET $URL Expected $EXPECTED_STATUS but got $RESPONSE"
        TEST_FAILED=1
    fi
}

test_post() {
    URL=$1
    EXPECTED_STATUS=$2
    DATA=$3
    RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -H "Content-Type: application/json" -d "$DATA" -X POST http://$HOST:$PORT$URL)
    if [ "$RESPONSE" == "$EXPECTED_STATUS" ]; then
        echo "OK: POST $URL"
    else
        echo "!!!KO!!!: POST $URL Expected $EXPECTED_STATUS but got $RESPONSE"
        TEST_FAILED=1
    fi
}

test_get_ipv6() {
    URL=$1
    EXPECTED_STATUS=$2
    RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -g "http://[$HOST_IPV6]:$PORT$URL")
    if [ "$RESPONSE" == "$EXPECTED_STATUS" ]; then
        echo "OK: GET IPv6 $URL"
    else
        echo "!!!KO!!!: GET IPv6 $URL Expected $EXPECTED_STATUS but got $RESPONSE"
        TEST_FAILED=1
    fi
}

test_post_ipv6() {
    URL=$1
    EXPECTED_STATUS=$2
    DATA=$3
    RESPONSE=$(curl -s -o /dev/null -w "%{http_code}" -H "Content-Type: application/json" -d "$DATA" -X POST http://$HOST:$PORT$URL)
    if [ "$RESPONSE" == "$EXPECTED_STATUS" ]; then
        echo "OK: POST IPv6 $URL"
    else
        echo "!!!KO!!!: POST IPv6 $URL Expected $EXPECTED_STATUS but got $RESPONSE"
        TEST_FAILED=1
    fi
}

test_cgi_hello() {
    URL="/cgi/hello.py"
    EXPECTED_STATUS="200"
    EXPECTED_CONTENT_TYPE="text/html"
    RESPONSE=$(curl -i -s -o response.txt -w "%{http_code}" http://$HOST:$PORT$URL)
    CONTENT_TYPE=$(grep 'Content-Type' response.txt | cut -d ':' -f2- | tr -d ' \r\n\t')
    if [[ "$RESPONSE" == "$EXPECTED_STATUS" && "$CONTENT_TYPE" == *"$EXPECTED_CONTENT_TYPE"* ]]; then
        echo "OK: GET $URL"
    else
        echo "!!!KO!!!: GET $URL Expected $EXPECTED_STATUS and $EXPECTED_CONTENT_TYPE but got $RESPONSE and $CONTENT_TYPE"
        TEST_FAILED=1
    fi
    rm response.txt
}

return_failure_if_test_fails() {
    if [ "$TEST_FAILED" -ne 0 ]; then
        printf "\n!!!KO!!!: One or more tests failed.\n\n"
        exit 1
    fi
}

# Run tests
test_get "/" "200"
test_get "/nonexistent" "404"
test_get "/cgi/hello.py" "200"
test_get "/cgi/hello.php" "200"
test_get "/cgi/off/disabled.py" "403"
test_post "/cgi/post.py" "200" '{"name": "test"}'
test_post "/cgi/post.php" "200" '{"name": "test"}'
test_post "/cgi/limited/post.py" "413" '{"name": "too loooooooooooooooooooooooooooooooooooooooooooooong"}'
test_get_ipv6 "/" "200"
test_get_ipv6 "/nonexistent" "404"
test_cgi_hello

# Check if any test failed
return_failure_if_test_fails
