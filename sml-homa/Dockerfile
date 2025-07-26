# https://developers.home-assistant.io/docs/add-ons/configuration#add-on-dockerfile
ARG BUILD_ARCH=amd64
#ARG BUILD_FROM=ghcr.io/home-assistant/${BUILD_ARCH}-base-debian:bookworm
ARG BUILD_FROM=ghcr.io/home-assistant/amd64-base:latest
FROM $BUILD_FROM

ENV LIBSML_VERSION="v1.1.3"

# Install dependencies
RUN \
    apk update && \
    apk upgrade && \
    apk add --no-cache mosquitto-clients yaml-cpp libuuid

COPY bashrc /root/.bashrc
COPY sml2mqtt /root/src/sml2mqtt

# Compile and install libsml and sml2mqtt
RUN \
    apk add --no-cache build-base git cmake util-linux-dev mosquitto-dev yaml-cpp-dev && \
    cd /root/src && \
    git clone --depth 1 --branch $LIBSML_VERSION https://github.com/volkszaehler/libsml.git && \
    cd libsml && \
    make prefix=/usr/local install && \
    mkdir -p /usr/local/lib/pkgconfig && \
    cp sml.pc /usr/local/lib/pkgconfig && \
    cd ../sml2mqtt && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local .. && \
    make install && \
    cd / && \
    apk del --purge build-base git cmake util-linux-dev yaml-cpp-dev && \
    rm -rf /tmp/* /var/tmp/* /usr/local/lib/pkgconfig /root/src

# Copy root filesystem
COPY rootfs /
