FROM alpine:3.20
RUN apk add --no-cache make clang
RUN ln -sf /usr/bin/clang++ /usr/bin/c++ && mkdir -p /var/www/html
COPY ./run.sh /app/
COPY ./src/ ./Makefile ./default.conf /app/
WORKDIR /app
RUN make debug
ENTRYPOINT [ "./run.sh" ]
CMD ["./webserv"]
