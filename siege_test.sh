#!/bin/bash

SIEGE_URLS_FILE="siegeurls.txt"
SIEGE_LOG_FILE="siege.log"
TEST_FAILED=0

cat <<EOL > $SIEGE_URLS_FILE
http://${CONTAINER}:${PORT}/
http://${CONTAINER}:8090/
http://${CONTAINER}:8091/
http://${CONTAINER}:8092/
http://${CONTAINER}:8093/
http://${CONTAINER}:${PORT}/nonexistent
http://${CONTAINER}:${PORT}/cgi/hello.py?name=Marvin
http://${CONTAINER}:${PORT}/cgi/hello.php?name=Marvin
http://${CONTAINER}:${PORT}/cgi/hello.js?name=Marvin
http://${CONTAINER}:${PORT}/cgi/post.py POST {"name": "Marvin"}
http://${CONTAINER}:${PORT}/cgi/post.php POST {"name": "Marvin"}
http://${CONTAINER}:${PORT}/cgi/post.js POST {"name": "Marvin"}
EOL

cat $SIEGE_URLS_FILE > $SIEGE_LOG_FILE
printf "\n" >> $SIEGE_LOG_FILE

siege > /dev/null 2>&1

echo "Running siege with no-cache"
echo "Running siege with no-cache" >> $SIEGE_LOG_FILE
siege -t 10s -c 250 -b -f $SIEGE_URLS_FILE -H "Cache-Control: no-cache" >> $SIEGE_LOG_FILE

if grep -q '"failed_transactions": *[1-9]' $SIEGE_LOG_FILE; then
    echo "!!!KO!!!: Siege test failed with no-cache. Check $SIEGE_LOG_FILE for details."
    TEST_FAILED=1
else
    echo "OK: Siege test passed with no-cache."
fi
printf "\n"

sleep 2

echo "Running siege with caching enabled"
echo "Running siege with caching enabled" >> $SIEGE_LOG_FILE
siege -t 10s -c 250 -b -f $SIEGE_URLS_FILE >> $SIEGE_LOG_FILE

if grep -q '"failed_transactions": *[1-9]' $SIEGE_LOG_FILE; then
    echo "!!!KO!!!: Siege test failed with caching enabled. Check $SIEGE_LOG_FILE for details."
    TEST_FAILED=1
else
    echo "OK: Siege test passed with caching enabled."
fi
printf "\n"

if [ "$TEST_FAILED" -ne 0 ]; then
    printf "\n!!!KO!!!: One or more tests failed.\n\n"
    exit 1
fi
printf "\n"
