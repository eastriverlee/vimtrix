SRCS=vimtrix.c\
	 motion.c\
	 util.c
OBJS=$(SRCS:.c=.o)
NAME=vimtrix
PACKAGE=$(NAME).app/Contents

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(OBJS) `sdl2-config --libs` -lSDL2_ttf
	mv $(NAME) $(PACKAGE)/MacOS/

$(OBJS): $(SRCS)
	$(CC) -c $(SRCS) `sdl2-config --cflags`

clean:
	rm $(OBJS)

re: clean $(NAME)

.PHONY: clean fclean re
