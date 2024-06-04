#!/bin/sh

if [ ! -z "$MY_UID" ] && [ ! -z "$MY_GID" ]; then
  chown -R $MY_UID:$MY_GID /var/www/html
  chown -R $MY_UID:$MY_GID /app
fi

make -C /app debug

exec "$@"
