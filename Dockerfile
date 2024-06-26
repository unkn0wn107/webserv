FROM alpine:3.20

RUN apk add --no-cache make nodejs clang python3 php82-cgi inotify-tools curl
RUN ln -sf /usr/bin/clang++ /usr/bin/c++ && mkdir -p /var/www/html \
    && ln -sf /usr/bin/php-cgi82 /usr/bin/php-cgi

RUN mkdir -p /var/www/html

ARG UID
ARG GID
RUN export MY_UID=${UID} && export MY_GID=${GID}

WORKDIR /app

COPY ./run.sh .

HEALTHCHECK --interval=1s --timeout=60s \
  CMD [ -f /tmp/healthy ] || (curl -f http://localhost:8080/ && touch /tmp/healthy || exit 1)

ENTRYPOINT [ "./run.sh" ]
CMD ["./webserv"]
