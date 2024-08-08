FROM ubuntu:latest
RUN apt-get update && apt-get install -y \
    build-essential \
    gdb \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . /app
RUN gcc -g -o linux_main_tests main-tests.c master-main.c data_structures/a_master_map.c data_structures/dynamic_hash_map_string.c data_structures/dynamic_hash_map_string_array.c
ENTRYPOINT ["/bin/bash"]