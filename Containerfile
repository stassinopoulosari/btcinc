FROM alpine
RUN apk add gcc gmp gmp-dev libc-dev make
COPY . ./btcinc
