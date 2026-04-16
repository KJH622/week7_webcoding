FROM gcc:14

WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends make \
    && rm -rf /var/lib/apt/lists/*

COPY . .

RUN make clean || true
RUN make all

CMD ["sh", "-c", "./bin/datagen 1000000 && ./bin/bench"]
