CC = cc

CFLAGS = -Wall -Wextra -Werror
LDLIBS = -lm
CPPFLAGS = -Ilib

SRC_DIR = src
OBJ_DIR = .obj

SRCS =	$(SRC_DIR)/args.c \
		$(SRC_DIR)/main.c \
		$(SRC_DIR)/signal.c \
		$(SRC_DIR)/socket.c \
		$(SRC_DIR)/packet.c \
		$(SRC_DIR)/receive.c \
		$(SRC_DIR)/network.c \
		$(SRC_DIR)/utils.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = icmp-ping

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re
