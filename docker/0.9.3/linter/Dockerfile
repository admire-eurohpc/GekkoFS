FROM gekkofs/core:0.9.3-rc1

LABEL Description="Debian-based environment to check the formatting of GekkoFS code"

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
		# clang 15 deps for clang-format
		lsb-release \
		wget \
		software-properties-common \
		gnupg2 && \
    # add clang-15 repos
    wget https://apt.llvm.org/llvm.sh -P /tmp && chmod +x /tmp/llvm.sh && /tmp/llvm.sh 15 && \
    # install clang-format
    apt-get update && apt-get install -y --no-install-recommends clang-format-15 && \
    # Clean apt cache to reduce image layer size
    rm -rf /var/lib/apt/lists/* && rm /tmp/llvm.sh && \
    # Clean apt caches of packages
    apt-get clean && apt-get autoclean
