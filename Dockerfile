# Conch Shell Docker Image
FROM gcc:latest

# Install dependencies
RUN apt-get update && apt-get install -y \
    make \
    valgrind \
    cppcheck \
    clang-format \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the shell
RUN make clean && make release

# Set the shell as entrypoint
ENTRYPOINT ["./bin/conch"]

# Add metadata
LABEL maintainer="SOLARIS"
LABEL description="Conch - a Unix shell written in C"
LABEL version="3.0"
