FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    unzip \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    libgl1-mesa-dev \
    && rm -rf /var/lib/apt/lists/*


WORKDIR /app

COPY . /app

RUN mkdir -p build
WORKDIR /app/build

RUN cmake .. -DCMAKE_BUILD_TYPE=Release

RUN cmake --build . -- -j$(nproc)


CMD ["./AssociationRules"]