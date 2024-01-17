INSTALL_DIR := /usr/bin
EXEC_NAME := 2048

all: $(EXEC_NAME)

$(EXEC_NAME): main.c
	$(CC) -o $(EXEC_NAME) -lncurses -lm main.c -std=c99

.PHONY: install clean

install: $(EXEC_NAME)
	sudo install -m 755 $< $(INSTALL_DIR)/$(EXEC_NAME)

clean:
	rm $(EXEC_NAME)
