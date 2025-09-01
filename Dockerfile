# Imagen base de la cátedra
FROM --platform=linux/amd64 agodio/itba-so-multi-platform:3.0

# Evitar prompts interactivos
ARG DEBIAN_FRONTEND=noninteractive

# 1) Instala dependencias necesarias + libncurses-dev + valgrind
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libncurses-dev \
        ca-certificates \
        wget \
        gnupg \
        strace \
        build-essential \
        cmake \
        valgrind \
        bear && \
    rm -rf /var/lib/apt/lists/*

# 2) Agrega el repo oficial de PVS-Studio e instala
RUN wget -qO- https://files.pvs-studio.com/etc/pubkey.txt \
      | gpg --dearmor -o /etc/apt/trusted.gpg.d/viva64.gpg && \
    wget -O /etc/apt/sources.list.d/viva64.list \
      https://files.pvs-studio.com/etc/viva64.list && \
    apt-get update && \
    apt-get install -y --no-install-recommends pvs-studio && \
    rm -rf /var/lib/apt/lists/*

# 3) (Opcional) Configurar credenciales en build
ARG PVS_STUDIO_USER=""
ARG PVS_STUDIO_KEY=""
RUN if [ -n "$PVS_STUDIO_USER" ] && [ -n "$PVS_STUDIO_KEY" ]; then \
      pvs-studio-analyzer credentials "$PVS_STUDIO_USER" "$PVS_STUDIO_KEY"; \
    fi

WORKDIR /root

