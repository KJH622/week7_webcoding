CC = gcc
CFLAGS = -Wall -O2 -std=c99

SRC_DIR = src
BIN_DIR = bin

all: $(BIN_DIR) datagen bench shell search

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

datagen:
	$(CC) $(CFLAGS) $(SRC_DIR)/benchmark/datagen.c -o $(BIN_DIR)/datagen

bench:
	$(CC) $(CFLAGS) \
		$(SRC_DIR)/linear/linear_search.c \
		$(SRC_DIR)/btree/btree.c \
		$(SRC_DIR)/bplus_tree/bplus_tree.c \
		$(SRC_DIR)/benchmark/benchmark.c \
		-o $(BIN_DIR)/bench

shell:
	$(CC) $(CFLAGS) \
		$(SRC_DIR)/linear/linear_search.c \
		$(SRC_DIR)/btree/btree.c \
		$(SRC_DIR)/bplus_tree/bplus_tree.c \
		$(SRC_DIR)/shell.c \
		-o $(BIN_DIR)/shell

search:
	$(CC) $(CFLAGS) \
		$(SRC_DIR)/linear/linear_search.c \
		$(SRC_DIR)/btree/btree.c \
		$(SRC_DIR)/bplus_tree/bplus_tree.c \
		$(SRC_DIR)/search.c \
		-o $(BIN_DIR)/search

run:
	./$(BIN_DIR)/datagen 1000000
	./$(BIN_DIR)/bench

result:
	@echo "===== 벤치마크 결과 ====="
	./$(BIN_DIR)/bench | column -t

clean:
	rm -rf $(BIN_DIR)

.PHONY: all datagen bench shell search run result clean
