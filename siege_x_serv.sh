#!/bin/bash

SIEGE_URLS_FILE="siegeurls.txt"
SIEGE_LOG_FILE="siege.log"
TEST_FAILED=0

PORT=8080
ADDR=localhost

cat <<EOL > $SIEGE_URLS_FILE
http://${ADDR}:${PORT}/
http://${ADDR}:8090/
http://${ADDR}:8091/
http://${ADDR}:8092/
http://${ADDR}:8093/
EOL

cat $SIEGE_URLS_FILE > $SIEGE_LOG_FILE
printf "\n" >> $SIEGE_LOG_FILE

siege > /dev/null 2>&1

echo "Running siege without caching"
echo "Running siege without caching" >> $SIEGE_LOG_FILE
siege -v -t 10s -c 25 -b -H "Cache-Control: no-cache" -f $SIEGE_URLS_FILE >> $SIEGE_LOG_FILE

if grep -q '"failed_transactions": *[1-9]' $SIEGE_LOG_FILE; then
    echo "!!!KO!!!: Siege test failed without caching. Check $SIEGE_LOG_FILE for details."
    TEST_FAILED=1
else
    echo "OK: Siege test passed without caching."
fi
printf "\n"

if [ "$TEST_FAILED" -ne 0 ]; then
    printf "\n!!!KO!!!: One or more tests failed.\n\n"
    exit 1
fi
printf "\n"
