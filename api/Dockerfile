FROM alpine:3.17.0 AS build

WORKDIR /waifu_api

COPY cmake/ ./cmake/
COPY src/ ./src/
COPY CMakeLists.txt .

RUN apk update && apk add --no-cache \
    build-base=0.5-r3 \
    cmake=3.24.4-r0 \
    git=2.38.5-r0 \
    openssl-dev=3.0.12-r0 \
    && cmake -S . -B build && cmake --build build --config Release

FROM alpine:3.17.0

RUN apk update && apk add --no-cache \
    libstdc++=12.2.1_git20220924-r4 \
    openssl-dev=3.0.12-r0 \
    && addgroup -S nonroot && adduser -S nonroot -G nonroot

USER nonroot

WORKDIR /app

COPY --from=build /waifu_api/build/_deps/net-build/libnet.so /usr/local/lib/
COPY --from=build --chown=nonroot:nonroot /waifu_api/build/waifu_api .

EXPOSE 8080

ENTRYPOINT ["./waifu_api"]
