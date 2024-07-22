#!/bin/bash

SIEGE_CGI_URLS_FILE="urls_cgi.txt"
SIEGE_GET_URLS_FILE="urls_get.txt"
SIEGE_LOG_FILE="siege.log"
TEST_DURATION=20s
TEST_FAILED=0

cat <<EOL > $SIEGE_CGI_URLS_FILE
http://${CONTAINER}:${PORT}/cgi/hello.py?name=Marvin
http://${CONTAINER}:${PORT}/cgi/hello.php?name=Marvin
http://${CONTAINER}:${PORT}/cgi/hello.js?name=Marvin
http://${CONTAINER}:${PORT}/cgi/post.py POST {"name": "Marvin"}
http://${CONTAINER}:${PORT}/cgi/post.php POST {"name": "Marvin"}
http://${CONTAINER}:${PORT}/cgi/post.js POST {"name": "Marvin"}
EOL

cat <<EOF > $SIEGE_GET_URLS_FILE
http://${CONTAINER}:${PORT}/
http://${CONTAINER}:8090/
http://${CONTAINER}:8091/
http://${CONTAINER}:8092/
http://${CONTAINER}:8093/
EOF

echo "CGI urls :"
cat $SIEGE_CGI_URLS_FILE > $SIEGE_LOG_FILE
printf "\n" >> $SIEGE_LOG_FILE
echo "GET urls :"
cat $SIEGE_GET_URLS_FILE >> $SIEGE_LOG_FILE
printf "\n" >> $SIEGE_LOG_FILE


siege > /dev/null 2>&1

echo "Running siege with no-cache for CGIs"
echo "Running siege with no-cache for CGIs" >> $SIEGE_LOG_FILE
siege -t ${TEST_DURATION} -c 250 -b -f $SIEGE_CGI_URLS_FILE -H "Cache-Control: no-cache" >> $SIEGE_LOG_FILE

if grep -q '"failed_transactions": *[1-9]' $SIEGE_LOG_FILE; then
    echo "!!!KO!!!: Siege test failed with no-cache for CGIs."
    TEST_FAILED=1
else
    echo "OK: Siege test passed with no-cache for CGIs."
fi
printf "\n"

sleep 2

echo "Running siege with caching enabled for CGIs"
echo "Running siege with caching enabled for CGIs" >> $SIEGE_LOG_FILE
siege -t ${TEST_DURATION} -c 250 -b -f $SIEGE_CGI_URLS_FILE -H "Cookie: sessionid=marvin;" >> $SIEGE_LOG_FILE

if grep -q '"failed_transactions": *[1-9]' $SIEGE_LOG_FILE; then
    echo "!!!KO!!!: Siege test failed with caching enabled for CGIs."
    TEST_FAILED=1
else
    echo "OK: Siege test passed with caching enabled for CGIs."
fi
printf "\n"

sleep 2

echo "Running siege for GET single"
echo "Running siege for GET single" >> $SIEGE_LOG_FILE
siege -t ${TEST_DURATION} -c 250 -b -H "Cookie: sessionid=marvin;" http://${CONTAINER}:${PORT} >> $SIEGE_LOG_FILE

if grep -q '"failed_transactions": *[1-9]' $SIEGE_LOG_FILE; then
    echo "!!!KO!!!: Siege test failed for GET."
    TEST_FAILED=1
else
    echo "OK: Siege test passed for GET."
fi
printf "\n"

sleep 2

echo "Running siege for GET cross-servers"
echo "Running siege for GET cross-servers" >> $SIEGE_LOG_FILE
siege -t ${TEST_DURATION} -c 250 -b -f $SIEGE_GET_URLS_FILE -H "Cookie: sessionid=marvin;" >> $SIEGE_LOG_FILE

if grep -q '"failed_transactions": *[1-9]' $SIEGE_LOG_FILE; then
    echo "!!!KO!!!: Siege test failed for GET cross-servers."
    TEST_FAILED=1
else
    echo "OK: Siege test passed for GET cross-servers."
fi
printf "\n"

if [ "$TEST_FAILED" -ne 0 ]; then
    printf "\n!!!KO!!!: One or more tests failed.\n\n"
    exit 1
fi
printf "\n"
