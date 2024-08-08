FROM ubuntu:latest
RUN apt-get update && apt-get install -y \
    build-essential \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . /app
RUN gcc -o linux_main_tests main-tests.c master-main.c data_structures/a_master_map.c data_structures/dynamic_hash_map_string.c data_structures/dynamic_hash_map_string_array.c
CMD ["./linux_main_tests"]
