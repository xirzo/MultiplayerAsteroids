FROM alpine:latest AS build

RUN apk update && \
    apk add --no-cache \
    cmake \
    git \
    build-base \
    raylib-dev

RUN git clone https://github.com/xirzo/logger \
    /tmp/logger && \
    mkdir -p /tmp/logger/build && \
    cd /tmp/logger/build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local .. && \
    cmake --build . && \
    cmake --install .

WORKDIR /asteroids
COPY bin/ ./bin/
COPY src/ ./src/
COPY CMakeLists.txt .

WORKDIR /asteroids/build
RUN cmake .. && \
    cmake --build . 

FROM alpine:latest

RUN addgroup -S asteroids && \
    adduser -S asteroids -G asteroids

WORKDIR /app

USER asteroids

COPY --chown=asteroids:asteroids --from=build \
    ./asteroids/build/bin/asteroids_server \ 
    ./

ENTRYPOINT ["./asteroids_server"]
