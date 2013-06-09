#include <stdio.h>
#include "enet/enet.h"
#include "debug.h"
#include "common.h"

#undef          DEBUG
#define         DEBUG           1

struct client_info_t
{
        char name[8];                  /* ipcam name */
        int type;                               /* type: CONTROL or STREAM */
        int port;
        char ip[32];
}client_info;

struct MEM_ITEM
{
	void 			*memory;
	int				 use;
};

/* 16/32/64/128/256/512/1024 */
#define MEM_TYPE_COUNT      7
#define PER_MEM_COUNT     512
static int MemType[MEM_TYPE_COUNT]   ={16,     32,  64, 128, 256, 512, 1024};
static int MEM_COUNT[MEM_TYPE_COUNT] ={2048, 1024, 512, 1024, 512, 512, 512};  //total 1056KB
static unsigned long MemBase[MEM_TYPE_COUNT];
static struct MEM_ITEM* MemItem[MEM_TYPE_COUNT];
static unsigned char MEM_BLOCK[1124*1024];
static unsigned long counter1024=0;

static void InitMemManager()
{
	int i, j;

	for(i=0; i < MEM_TYPE_COUNT; i++)
		MemItem[i] = (struct MEM_ITEM *)malloc(sizeof(struct MEM_ITEM)*MEM_COUNT[i]);

	//calc every base
	MemBase[0] = (unsigned long)(&MEM_BLOCK[0]);
	MemBase[1] = MemBase[0] + MEM_COUNT[0]*16;
	MemBase[2] = MemBase[1] + MEM_COUNT[1]*32;
	MemBase[3] = MemBase[2] + MEM_COUNT[2]*64;
	MemBase[4] = MemBase[3] + MEM_COUNT[3]*128;
	MemBase[5] = MemBase[4] + MEM_COUNT[4]*256;
	MemBase[6] = MemBase[5] + MEM_COUNT[5]*512;

	//init every item
	for(i = 0; i < MEM_TYPE_COUNT; i++)
	{
		for(j = 0; j < MEM_COUNT[i]; j++)
		{
			MemItem[i][j].memory = MemBase[i] + j * MemType[i];
			MemItem[i][j].use=0;
		}
	}
}

static void * my_malloc(size_t size)
{
	int type;
	int i;
	if(size > 1024) 
	{
		printf("Warning: my_malloc of size(%d) overflow !!!!!!!!!!!!!\n", size);
		return malloc(size);
	}

	//index = (size-1) / 16;
	for(i=0; i < MEM_TYPE_COUNT; i++)
	{
		if(MemType[i] >= size) 
		{
			type = i;
			break;	
		}
	}
	//printf("my_malloc size=%d, index=%d\n", size, index);
	for(i=0; i < MEM_COUNT[type]; i++)
	{
		if(MemItem[type][i].use == 0)
		{
			MemItem[type][i].use = 1;
			//if(type==6) printf("malloc 1024 counter=%d\n", ++counter1024);
			return MemItem[type][i].memory;
		}
	}

	printf("Error: my_malloc of size(%d) is exhaust !!!!!!!!!!!!!!\n", size);
	return NULL;
}

static void CHECK_INDEX(int type, int index)
{
	if(index >= MEM_COUNT[type])
		printf("ERROR!!!!!!!!!!!!!!!!!!! CHECK_INDEX %d\n", index);
	if(MemItem[type][index].use == 0)
		printf("ERROR!!!!!!!!!!CHECK_INDEX[%d][%d] is already unused......\n");
}

static void my_free(void *memory)
{
	unsigned long free_mem;
	int index;
	free_mem = (unsigned long)memory;
	if(free_mem >= MemBase[6])
	{
		index = (free_mem - MemBase[6]) / 1024;
		CHECK_INDEX(6, index);
		MemItem[6][index].use = 0;
		//printf("free 1024 counter=%d\n", --counter1024);
	}
	else if(free_mem >= MemBase[5])
	{
		index = (free_mem - MemBase[5]) / 512;
		CHECK_INDEX(5, index);
		MemItem[5][index].use = 0;
	}
	else if(free_mem >= MemBase[4])
	{
		index = (free_mem - MemBase[4]) / 256;
		CHECK_INDEX(4, index);
		MemItem[4][index].use = 0;
	}
	else if(free_mem >= MemBase[3])
	{
		index = (free_mem - MemBase[3]) / 128;
		CHECK_INDEX(3, index);
		MemItem[3][index].use = 0;
	}
	else if(free_mem >= MemBase[2])
	{
		index = (free_mem - MemBase[2]) / 64;
		CHECK_INDEX(2, index);
		MemItem[2][index].use = 0;
	}
	else if(free_mem >= MemBase[1])
	{
		index = (free_mem - MemBase[1]) / 32;
		CHECK_INDEX(1, index);
		MemItem[1][index].use = 0;
	}
	else if(free_mem >= MemBase[0])
	{
		index = (free_mem - MemBase[0]) / 16;
		CHECK_INDEX(0, index);
		MemItem[0][index].use = 0;
	}
	else
	{
		printf("Error: my_free memory %ld not in scope !!!!!!!!!\n", free_mem);
	}
}
/* server receive packet */
/*      4      |      8     |       4      */
/*  from_ipcam | ipcam_name | channel_type */
/*    from_pc  | ipcam_name | channel_type */

/* server send packet */
/*      32    |       4     |     4        */
/*  ipcam_ip  |  ipcam_port | channel_type */


static ENetCallbacks my_callbacks = { my_malloc, my_free, rand };

int main(int argc, char argv[])
{
        ENetAddress native_addr;
        ENetHost *center_host;
        ENetEvent event;     

#if 1
        strcpy(client_info.name, "jason");
        strcpy(client_info.ip, "127.0.0.1");
        client_info.port = 8888;
#endif

#if 0
        if (enet_initialize() < 0)
        {
                MY_PRINTF("enet lib init faile\n");
                return -1;
        }     
#else
	InitMemManager();
	if(enet_initialize_with_callbacks(ENET_VERSION, &my_callbacks) != 0)
	{
		printf("An error occured while initializing enet.\n");
		return;
	}
#endif

        native_addr.host=ENET_HOST_ANY;
        native_addr.port=1234;

        center_host = enet_host_create(&native_addr, 64, 0, 0);                         /* 允许64个连接 */
        if (center_host == NULL)
        {
                MY_PRINTF("enet_host_create error\n");
                return -1;
        }

#if 0
        if (enet_host_service(center_host, &event, 5000) >= 0) && (event.type == ENET_EVENT_TYPE_CONNECT))  
        {
                char ip[256];    

                remote = event.peer->address; //远程地址  
                enet_address_get_host_ip(&remote, ip, 256);
                MY_PRINTF("remote url %s:%d\n", ip, remote.port);
        }
#endif        

        while (enet_host_service(center_host, &event, 200) >= 0 )
        {

              
                if(event.type == ENET_EVENT_TYPE_CONNECT) //有客户机连接成功
                {
                        char ip[256];    
                        ENetAddress remote = event.peer->address; //远程地址
                        
                        enet_address_get_host_ip(&remote,ip,256);
                        printf("remote url %s:%d\n", ip, remote.port);
                }
                else if(event.type==ENET_EVENT_TYPE_RECEIVE) //收到数据
                {

                       ENetAddress remote = event.peer->address; //远程地址
                       
  //                      printf("recv data:%s\n", (char *)event.packet->data);
                        if ( *(int *)event.packet->data == FROM_IPCAM)
                        {
                                memcpy(client_info.name, (char *)(event.packet->data + 4), 8);
                                memcpy(&client_info.type, (char *)(event.packet->data + 12), 4);
                                client_info.port = remote.port;
                                enet_address_get_host_ip(&remote, client_info.ip, 256);
                                enet_packet_destroy(event.packet);    //注意释放空间

                                printf("channel info:\tname:%s, type:%s, ip:%s, port:%d\n", client_info.name, client_info.type ? "STREAM_TYPE" : "CONTROL_TYPE",
                                                                                                                        client_info.ip, client_info.port);
                        }
                        else
                        {
                                char send_buf[40];
                                ENetPacket *packet;

                                memcpy(client_info.name, (char *)(event.packet->data + 4), 8);
                                memcpy(&client_info.type, (char *)(event.packet->data + 12), 4);
                                enet_packet_destroy(event.packet);    //注意释放空间                                
                                printf("from pc request info:\tname:%s, type:%s\n", client_info.name, client_info.type ? "STREAM_TYPE" : "CONTROL_TYPE");
                                
                                strcpy(send_buf, client_info.ip);
                                memcpy(send_buf + 32, &client_info.port, 4);
                                memcpy(send_buf + 36, &client_info.type, 4);

                                packet=enet_packet_create(NULL, 40, ENET_PACKET_FLAG_RELIABLE);                 /* 创建包 */
                                memcpy((char*)packet->data, send_buf, sizeof(send_buf));
                                if (enet_peer_send(event.peer, 1, packet) < 0)
                                        MY_PRINTF("enet_peer_send fail\n");                                              
                        }
                }
                else if(event.type==ENET_EVENT_TYPE_DISCONNECT) //失去连接
                {
                        printf("connect is down port %d\n", event.peer->address.port);
                }
        }

        return 0;
}
