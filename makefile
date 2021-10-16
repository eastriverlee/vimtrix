SRCS=*.c
OBJS=$(SRCS:.c=.o)
NAME=vimtrix

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(SRCS)
clean:
	rm $(OBJS)
fclean: clean
	rm $(NAME)

.PHONY: clean fclean
