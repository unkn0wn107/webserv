#!/bin/sh

if [ ! -z "$MY_UID" ] && [ ! -z "$MY_GID" ]; then
  chown -R $MY_UID:$MY_GID /var/www/html
  chown -R $MY_UID:$MY_GID /app
fi

if [ "$BUILD_TYPE" = "debug" ]; then
  make -C /app debug
else
  make -C /app
fi

exec "$@"
