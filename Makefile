OBJS1	=	server_main.o server_net.o server_command.o
OBJS2	=	client_main.o client_net.o client_command.o client_win.o
TARGET1	=	server
TARGET2	=	client
CFLAGS	=	-c -DNDEBUG

.c.o:
	gcc $(CFLAGS) $<

all: $(TARGET1) $(TARGET2)

$(TARGET1):	$(OBJS1)
	gcc -o $(TARGET1) $(OBJS1) -lm -lSDL2
$(TARGET2):	$(OBJS2)
	gcc -o $(TARGET2) $(OBJS2) -lm -lSDL2 -lSDL2_image -lSDL2_gfx -ljoyconlib -lhidapi-hidraw -lSDL2_mixer $(LDFLAGS)

clean:
	rm *.o $(TARGET1) $(TARGET2)

server_main.o: server_main.c server_common.h server_func.h common.h
server_net.o: server_net.c server_common.h server_func.h common.h
server_command.o: server_command.c server_common.h server_func.h common.h
client_main.o: client_main.c client_func.h common.h
client_net.o: client_net.c client_func.h common.h
client_command.o: client_command.c client_func.h common.h
client_win.o: client_win.c client_func.h common.h
