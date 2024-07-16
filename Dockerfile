FROM debian:bookworm-slim
RUN apt-get update
RUN apt-get install -y curl
RUN apt-get install -y make clang python3 php-cgi libpthread-stubs0-dev nodejs valgrind

RUN apt-get install -y git

RUN git clone --depth=1 https://github.com/unkn0wn107/website-examples.git /var/www/

RUN mkdir -p /var/www/html

WORKDIR /app

COPY ./run.sh .

HEALTHCHECK --interval=1s --timeout=60s \
  CMD [ -f /tmp/healthy ] || (curl -f http://localhost:8080/ && touch /tmp/healthy || exit 1)

ENTRYPOINT [ "./run.sh" ]
CMD ["./webserv"]
# CMD ["/bin/sh", "-c", "ulimit -n 2048 && ./webserv"]

