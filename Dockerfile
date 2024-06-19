FROM alpine:3.20
RUN apk add --no-cache make clang python3 php82-cgi inotify-tools curl
RUN ln -sf /usr/bin/clang++ /usr/bin/c++ && mkdir -p /var/www/html \
    && ln -sf /usr/bin/php-cgi82 /usr/bin/php-cgi 

WORKDIR /app

COPY ./run.sh .

HEALTHCHECK --interval=30s --timeout=30s --start-period=5s --retries=3 \
  CMD curl -f http://localhost:8080/ || exit 1

ENTRYPOINT [ "./run.sh" ]
CMD ["./webserv"]
