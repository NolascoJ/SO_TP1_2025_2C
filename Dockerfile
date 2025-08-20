# Imagen base de la cátedra
FROM agodio/itba-so-multi-platform:3.0

# Instala la biblioteca libncurses-dev
RUN apt-get update && \
    apt-get install -y libncurses-dev

# Elimina los archivos temporales
RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/*

