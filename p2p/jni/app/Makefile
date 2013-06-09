SERVER = center
IPC = ipc_client
PC = pc_client
BIN_DIR = ./../bin
COMMON_SRC = $(wildcard ./common/*.c)
SERVER_SRC = $(COMMON_SRC) ./server/server.c
SERVER_OBJ = $(patsubst %.c, %.o, $(SERVER_SRC))
IPC_SRC = $(COMMON_SRC) ./ipc/ipc.c
IPC_OBJ = $(patsubst %.c, %.o, $(IPC_SRC))
PC_SRC = $(COMMON_SRC) ./pc/pc.c
PC_OBJ = $(patsubst %.c, %.o, $(PC_SRC))

CROSS_COMPILE =

AS		= $(CROSS_COMPILE)as
AR		= $(CROSS_COMPILE)ar
CC		= $(CROSS_COMPILE)gcc -g -I./../include
CPP		= $(CC) -E -g -I./../include
LD		= $(CROSS_COMPILE)ld
NM		= $(CROSS_COMPILE)nm
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB	= $(CROSS_COMPILE)ranlib
READELF	= $(CROSS_COMPILE)readelf
SIZE	= $(CROSS_COMPILE)size
STRINGS = $(CROSS_COMPILE)strings
STRIP	= $(CROSS_COMPILE)strip
LIBS = -lcrypt -lpthread

all: $(SERVER) $(IPC) $(PC)

$(SERVER): $(SERVER_OBJ)
	echo $(SERVER_OBJ)
	$(CC) -o $@ $(SERVER_OBJ) $(LIBS)
	-cp -f $(SERVER) $(BIN_DIR)

$(IPC): $(IPC_OBJ)
	$(CC) -o $@ $(IPC_OBJ) $(LIBS)
	-cp -f $(IPC) $(BIN_DIR)	

$(PC): $(PC_OBJ)
	$(CC) -o $@ $(PC_OBJ) $(LIBS)
	-cp -f $(PC) $(BIN_DIR)		

clean:
	-rm -f $(BIN_DIR)/$(SERVER) $(SERVER) *.elf *.gdb *.o
	-rm -f $(BIN_DIR)/$(IPC) $(IPC) *.elf *.gdb *.o	
	-rm -f $(BIN_DIR)/$(PC) $(PC) *.elf *.gdb *.o		
	rm -f ./*.o
