FROM alpine
RUN apk add gcc gdb gmp gmp-dev libc-dev make
COPY . .
