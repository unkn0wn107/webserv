FROM alpine:3.20
RUN apk add --no-cache make clang
RUN ln -sf /usr/bin/clang++ /usr/bin/c++ && c++ --version
COPY ./src ./Makefile ./default.conf /app
WORKDIR /app
CMD ["./run_debug.sh"]
