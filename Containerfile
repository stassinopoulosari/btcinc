FROM alpine
RUN apk add gcc libc-dev
COPY . ./btcinc
