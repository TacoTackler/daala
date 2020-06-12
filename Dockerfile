# build envirnment
FROM ubuntu:18.04 AS build

# environment variables
ENV \
    APP_DIR=/opt/app \
    LC_ALL=C.UTF-8 \
    LANG=C.UTF-8 \
    LANGUAGE=C.UTF-8 \
    TZ=UTC \
    DEBIAN_FRONTEND=noninteractive
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# install base build dependencies and useful packages
RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        autoconf \
        automake \
        build-essential \
        ca-certificates \
        check \
        git \
        libicu-dev \
        libjpeg-dev \
        libogg-dev \
        libpng-dev \
        libtool \
        pkg-config \
        && \
    apt-get clean

# install daalatool
ENV \
    DAALATOOL_DIR=/opt/daalatool
RUN \
    mkdir -p $(dirname ${DAALATOOL_DIR}) && \
    git clone https://github.com/edmond-zhu/daala.git ${DAALATOOL_DIR} && \
    cd ${DAALATOOL_DIR} && \
    ./autogen.sh && \
    ./configure --disable-player && \
    make tools -j4 && \
    make install DESTDIR=/home/build && \
    make install


# runtime environment
FROM ubuntu:18.04

ENV \
    TZ=UTC \
    DEBIAN_FRONTEND=noninteractive
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends && \
    apt-get clean

# install dump_ciede2000
COPY --from=build /home/build /
COPY --from=build /home/build /home/build
ENV LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/local/lib:/usr/local/lib/x86_64-linux-gnu
