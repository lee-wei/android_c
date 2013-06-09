#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "enet/enet.h"
#include "debug.h"
#include "common.h"

#undef          DEBUG
#define         DEBUG           1


#if 0
typedef struct CLIENT_T
{
        char ip[32];
        ENetPeer *peer;
        struct CLIENT_T next;
}CLIENT;


#define         STREAM_PORT                     6666
#define         CONTROL_PORT                    (STREAM_PORT + 1)

struct CLIENT_T *first_stream_client = NULL;
struct CLIENT_T *first_control_client = NULL;

ENetAddress stream_host_addr;
ENetAddress control_host_addr;
stream_host_addr.host=ENET_HOST_ANY;
stream_host_addr.port=STREAM_PORT;
control_host_addr.host=ENET_HOST_ANY;
control_host_addr.port=CONTROL_PORT;

ENetHost *stream_server;
stream_server=enet_host_create(&stream_host_addr, 64, 0, 0);                     /* 允许64个连接数 */

ENetHost *control_server;
control_server=enet_host_create(&control_host_addr, 64, 0, 0);                     /* 允许64个连接数 */
#endif

#define         STREAM_PORT                     6666
#define         CONTROL_PORT                    (STREAM_PORT + 1)
//#define         CENTER_IP                       "127.0.0.1"
#define         CENTER_IP                       "113.90.168.67"
#define         PACKET_LEN                      16

struct session_info_t
{
        ENetPeer *peer;
        ENetHost *host;
}session_info;

static int control_flag = 0;
static int connect_flag = 0;

static ENetHost *control_host;
static ENetPeer *pc_peer; 

void register_thread(void)
{
//        ENetHost *control_host;
        ENetHost *stream_host;        
        ENetAddress native_control_addr;
        ENetAddress native_stream_addr;
        ENetAddress center_addr;        
        ENetPeer *stream_center_peer;
        ENetPeer *control_center_peer; 
        ENetEvent event;
        ENetPacket *packet;
        int packet_len = PACKET_LEN;

#if 0        
        native_control_addr.host = ENET_HOST_ANY;
        native_control_addr.port = CONTROL_PORT;
        native_stream_addr.host = ENET_HOST_ANY;
        native_stream_addr.port = STREAM_PORT;        
#endif        
        
        enet_address_set_host(&center_addr, CENTER_IP);     
        center_addr.port = 1267;     
 
        control_host = enet_host_create(NULL, 10, 0, 0);                         /* 只允许连接1个服务器 */
        stream_host = enet_host_create(NULL, 10, 0, 0);                         /* 只允许连接1个服务器 */ 

        stream_center_peer = enet_host_connect(stream_host, &center_addr, 10);                            /* client连接到svraddr对象，共分配三个通道  */
        if ((enet_host_service(stream_host,&event,5000 >= 0)) && (event.type == ENET_EVENT_TYPE_CONNECT))
                MY_PRINTF("connect remote stream server ok\n");
        else
        {
                MY_PRINTF("connect remote stream server fail\n");                
                return;
        }

        control_center_peer = enet_host_connect(control_host, &center_addr, 10);                            /* client连接到svraddr对象，共分配三个通道  */
        if ((enet_host_service(control_host, &event, 5000 >= 0)) && (event.type == ENET_EVENT_TYPE_CONNECT))
                MY_PRINTF("connect remote control server ok\n");        
        else
        {
                MY_PRINTF("connect remote control server fail\n");                
                return;
        }        

        while (1)
        {
                char send_buf[PACKET_LEN] = {0};
                enum server_request_t request = FROM_IPCAM; 
                enum channel_type_t channel = CONTROL_CHANNEL;                      

                memcpy(send_buf, &request, 4);
                strcpy(send_buf + 4, "jason");                
                memcpy(send_buf + 12, &channel, 4);
                
                packet=enet_packet_create(NULL, packet_len, ENET_PACKET_FLAG_RELIABLE);                 /* 创建包 */
                memcpy(packet->data, send_buf, packet_len);
                if (enet_peer_send(control_center_peer, 1, packet) < 0)
                        MY_PRINTF("enet_peer_send fail\n");                
                enet_host_service(control_host, &event, 1000);
                
#if 0
                packet=enet_packet_create(NULL, packet_len, ENET_PACKET_FLAG_RELIABLE);                 /* 创建包 */
                strcpy((char*)packet->data, "send control punching packet");
                if (enet_peer_send(control_center_peer, 1, packet) < 0)
                        MY_PRINTF("enet_peer_send fail\n");                
                enet_host_service(control_host, &event, 1000);
#endif

                session_info.host = control_host;
 
                control_flag = 1;
               
                sleep(1);
        }
}

#if 0
void stream_thread(void)                        /* 为每个客户开一个线程发送码流 */
{
        ENetEvent event;
        
        while(enet_host_service(server,&event,5000)>=0)
        {
                if(event.type==ENET_EVENT_TYPE_CONNECT) //有客户机连接成功
                {
                        static unsigned int num=0;
                        ENetAddress remote=event.peer->address; //远程地址
                        char ip[256];
                        enet_address_get_host_ip(&remote,ip,256);

                        event.peer->data=(void*)num++;
                }
                else if(event.type==ENET_EVENT_TYPE_RECEIVE) //收到数据
                {

                        enet_packet_destroy(event.packet);    //注意释放空间
                        cout <<endl;
                }
                else if(event.type==ENET_EVENT_TYPE_DISCONNECT) //失去连接
                {

                }
        }
}

#endif

void control_recv_thread(void)
{
        ENetEvent event;

        while (!control_flag) sleep(1);
        
        while(enet_host_service(session_info.host, &event, 1000) >= 0)
        {
                if(event.type==ENET_EVENT_TYPE_CONNECT) //有客户机连接成功
                {
                        ENetAddress remote = event.peer->address; //远程地址
                        char ip[256];
                        enet_address_get_host_ip(&remote,ip,256);
                        printf("remote url %s:%d\n", ip, remote.port);

                        connect_flag = 1;
                        session_info.peer = event.peer;
                }
                else if(event.type==ENET_EVENT_TYPE_RECEIVE) //收到数据
                {
                        printf("remote say:\t%s\n", event.packet->data);
                        enet_packet_destroy(event.packet);    //注意释放空间                                             
                }
                else if(event.type==ENET_EVENT_TYPE_DISCONNECT) //失去连接
                {
                        MY_PRINTF("connect is down\n");                  
                }
        }
}

void control_send_thread(void)
{
        int packet_len = 128;
        char send_buf[128] = {0};
        ENetPacket *packet; 

        while (!connect_flag) sleep(1);

        while(1)
        {
     //           scanf("%s", send_buf);
                sprintf(send_buf, "from pc .............\n");
                sleep(1);

                packet=enet_packet_create(NULL, packet_len, ENET_PACKET_FLAG_RELIABLE);                 /* 创建包 */
                memcpy(packet->data, send_buf, strlen(send_buf) + 1);
                if (enet_peer_send(session_info.peer, 1, packet) < 0)
                        MY_PRINTF("enet_peer_send fail\n");  

                usleep(100 * 1000);
        }
}

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

static ENetCallbacks my_callbacks = { my_malloc, my_free, rand };



int main(int argc, char *argv[])
{
        pthread_t stream_id, control_id, register_id;

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

        pthread_create(&stream_id, NULL, register_thread, (void *)NULL);
//        pthread_create(&stream_id, NULL, stream_thread, (void *)NULL);
        pthread_create(&stream_id, NULL, control_send_thread, (void *)NULL);   
        pthread_create(&stream_id, NULL, control_recv_thread, (void *)NULL);   

        while(1) sleep(1);

        return 0;
}
