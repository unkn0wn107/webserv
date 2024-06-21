FROM debian:bullseye-slim
RUN apt-get update
RUN apt-get install -y curl
RUN curl -fsSL https://deb.nodesource.com/setup_20.x -o nodesource_setup.sh && bash nodesource_setup.sh
RUN apt-get install -y make clang python3 php-cgi libpthread-stubs0-dev nodejs

RUN mkdir -p /var/www/html

WORKDIR /app

COPY ./run.sh .

HEALTHCHECK --interval=1s --timeout=60s \
  CMD [ -f /tmp/healthy ] || (curl -f http://localhost:8080/ && touch /tmp/healthy || exit 1)

ENTRYPOINT [ "./run.sh" ]
CMD ["./webserv"]
