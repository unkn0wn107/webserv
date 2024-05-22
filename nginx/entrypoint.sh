#!/bin/sh
chown -R nginx:nginx /var/www/html
exec nginx -g "daemon off;"
