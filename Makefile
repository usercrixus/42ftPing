OBJ = \
	srcs/ping.o \
	srcs/globals.o \
	srcs/signal.o \
	srcs/socket.o \
	srcs/receivePing.o \
	srcs/sendPing.o \
	main.o \

all: ft_ping

ft_ping: $(OBJ)
	gcc $(OBJ) -o $@  -lm

%.o: %.c
	gcc -Wall -Wextra -Werror -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f ft_ping

re: fclean all

.PHONY: clean fclean re