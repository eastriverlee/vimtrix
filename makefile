SRCS=vimtrix.c\
	 motion.c\
	 util.c
OBJS=$(SRCS:.c=.o)
NAME=vimtrix

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(OBJS) `sdl2-config --libs` -lSDL2_ttf

$(OBJS): $(SRCS)
	$(CC) -c $(SRCS) `sdl2-config --cflags`

clean:
	rm $(OBJS)
fclean: clean
	rm $(NAME)

.PHONY: clean fclean
