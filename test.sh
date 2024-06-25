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

test_cookie_session() {
    URL=$1
    SESSION_ID=$2
    RESPONSE=$(curl -i -s -o response.txt -H "Cookie: sessionid=$SESSION_ID; Path=/; HttpOnly" http://$HOST:$PORT$URL)
    SET_COOKIE=$(grep 'Set-Cookie' response.txt | grep -o "sessionid=[^;]*")
    if [[ "$SET_COOKIE" == "sessionid=$SESSION_ID" ]]; then
        echo "OK: Cookie session $URL"
    else
        echo "!!!KO!!!: Cookie session $URL Expected sessionid=$SESSION_ID but got $SET_COOKIE"
        TEST_FAILED=1
    fi
    rm response.txt
}

test_cache_control_no_cache() {
    URL=$1
    EXPECTED_STATUS=$2
    EXPECTED_CACHE_CONTROL="no-cache"
    RESPONSE=$(curl -i -s -o response.txt -H "Cache-Control: no-cache" http://$HOST:$PORT$URL)
    STATUS_CODE=$(grep 'HTTP/' response.txt | awk '{print $2}')
    CACHE_CONTROL=$(grep 'Cache-Control' response.txt | cut -d ':' -f2- | tr -d ' \r\n\t')
    if [[ "$STATUS_CODE" == "$EXPECTED_STATUS" && "$CACHE_CONTROL" == "$EXPECTED_CACHE_CONTROL" ]]; then
        echo "OK: Cache-Control no-cache $URL"
    else
        echo "!!!KO!!!: Cache-Control no-cache $URL Expected $EXPECTED_STATUS and $EXPECTED_CACHE_CONTROL but got $STATUS_CODE and $CACHE_CONTROL"
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
test_cookie_session "/" "jMErhrQmaTckyoO2eLDQNE3QAbwypuWG"
test_cache_control_no_cache "/" "200"

# Check if any test failed
return_failure_if_test_fails
