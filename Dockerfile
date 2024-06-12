FROM alpine:3.20
RUN apk add --no-cache make clang python3 php82-cgi inotify-tools
RUN ln -sf /usr/bin/clang++ /usr/bin/c++ && mkdir -p /var/www/html \
    && ln -sf /usr/bin/php-cgi82 /usr/bin/php-cgi 

WORKDIR /app

COPY ./run.sh .

ENTRYPOINT [ "./run.sh" ]
CMD ["./webserv"]
