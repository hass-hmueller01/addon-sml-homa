# https://developers.home-assistant.io/docs/add-ons/configuration#add-on-dockerfile
ARG BUILD_ARCH=amd64
ARG BUILD_FROM=ghcr.io/home-assistant/${BUILD_ARCH}-base:latest
FROM $BUILD_FROM

ENV LIBSML_VERSION="v1.1.3"

RUN echo "alias l='ls --color=auto -alF'" >> /root/.bashrc

# Install dependencies
RUN \
    apk update && \
    apk upgrade && \
    apk add --no-cache mosquitto-clients yaml-cpp libuuid git && \
    git -C /usr/local/src clone --depth 1 --branch $LIBSML_VERSION https://github.com/volkszaehler/libsml.git && \
    git -C /usr/local/src clone --depth 1 --branch main https://github.com/hass-hmueller01/hass-addon-helper.git

COPY sml2mqtt /usr/local/src/sml2mqtt

# Compile and install libsml, sml2mqtt, hass-addon-helper
RUN \
    apk add --no-cache build-base git cmake util-linux-dev mosquitto-dev yaml-cpp-dev && \
    cd /usr/local/src/libsml && \
    git pull && \
    make prefix=/usr/local install && \
    mkdir -p /usr/local/lib/pkgconfig && \
    cp sml.pc /usr/local/lib/pkgconfig && \
    cd /usr/local/src/sml2mqtt && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local .. && \
    make install && \
    cd /usr/local/src/hass-addon-helper && \
    git pull && \
    make install

# Cleanup
RUN \
    apk del --purge build-base git cmake util-linux-dev yaml-cpp-dev && \
    rm -rf /tmp/* /var/tmp/* /usr/local/lib/pkgconfig /usr/local/src/*

# Copy root filesystem
COPY rootfs /
