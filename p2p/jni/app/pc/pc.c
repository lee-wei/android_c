#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "enet/enet.h"
#include "debug.h"
#include "common.h"

#undef          DEBUG
#define         DEBUG           1

struct ipcam_url_t
{
        char    ip[32];
        int      port;
}ipcam_url;

struct session_info_t
{
        ENetPeer *peer;
        ENetHost *host;
}session_info;

void control_send_thread(void *args)
{
        struct session_info_t *control_session = (struct session_info_t *)args;
        ENetHost *native_host;
        char send_buf[128] = {0};
        int packet_len = 128;
        ENetPacket *packet; 
        ENetEvent event; 

        while (1)
        {
 //               scanf("%s", send_buf);
                sprintf(send_buf, "from ipcam .............\n");
                sleep(1);

                packet=enet_packet_create(NULL, packet_len, ENET_PACKET_FLAG_RELIABLE);                 /* 创建包 */
                memcpy(packet->data, send_buf, strlen(send_buf) + 1);
                if (enet_peer_send(control_session->peer, 1, packet) < 0)
                        MY_PRINTF("enet_peer_send fail\n");  
                enet_host_service(control_session->host, &event, 200);

                usleep(100 * 1000);
        }
}

#define         PACKET_LEN              128
void control_thread(void *args)
{
        ENetPeer *ipc_peer;
        ENetHost *native_host;
        ENetAddress ipc_addr;  
        ENetEvent event;   
        ENetPacket *packet; 
        struct ipcam_url_t *url;
        int packet_len = PACKET_LEN;
        char send_buf[PACKET_LEN] = {0};
        char recv_buf[PACKET_LEN] = {0}; 
        pthread_t control_send_id;  
        struct session_info_t  control_session;

        url = (struct ipcam_url_t *)args;
        
        enet_address_set_host(&ipc_addr, url->ip);     
        ipc_addr.port = url->port;                   

        native_host = enet_host_create(NULL, 10, 0, 0);                         /* 只允许连接1个服务器 */         

        ipc_peer = enet_host_connect(native_host, &ipc_addr, 10);                            /* client连接到svraddr对象，共分配三个通道  */
        if ((enet_host_service(native_host, &event, 200) >= 0) && (event.type == ENET_EVENT_TYPE_CONNECT))
                MY_PRINTF("connect remote stream server ok\n");
        else
        {
                MY_PRINTF("connect remote stream server fail\n");                
                return;
        }

        control_session.host = native_host;
        control_session.peer = ipc_peer;
        pthread_create(&control_send_id, NULL, control_send_thread, &control_session);
        
        while (1)
        {
                 if (enet_host_service(native_host, &event, 200)>=0)
                {
                        if(event.type == ENET_EVENT_TYPE_CONNECT) //有客户机连接成功
                        {
                                printf("success to connect server\n");
                        }
                        else if(event.type==ENET_EVENT_TYPE_RECEIVE) //收到数据
                        {
                                printf("remote say:\t%s\n", event.packet->data);
                        }
                        else if(event.type==ENET_EVENT_TYPE_DISCONNECT) //失去连接
                        {
                                printf("connect is down port %d\n", event.peer->address.port);
                        }
                }

                 usleep(100 * 1000);
        }

        return 0;
}

void stream_thread(void *args)
{

}

struct MEM_ITEM
{
        void   *memory;
        int     use;
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

//#define         CENTER_IP                       "127.0.0.1"
#define         CENTER_IP                       "113.90.168.67"
#define         PACKET_LEN                      16

int main(int argc, char *argv[])
{
        ENetPeer *center_peer;
        ENetHost *login_host;
        ENetAddress center_addr;  
        ENetEvent event;   
        ENetPacket *packet;        
        int packet_len = PACKET_LEN;
        pthread_t control_id, stream_id;


        enet_address_set_host(&center_addr, CENTER_IP);     
        center_addr.port = 1267;                   


        login_host = enet_host_create(NULL, 10, 0, 0);                         /* 只允许连接1个服务器 */ 

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

        center_peer = enet_host_connect(login_host, &center_addr, 10);                            /* client连接到svraddr对象，共分配三个通道  */
        if ((enet_host_service(login_host, &event, 5000 >= 0)) && (event.type == ENET_EVENT_TYPE_CONNECT))
        {
                char ip[256];    
                ENetAddress remote = event.peer->address; //远程地址
                
                enet_address_get_host_ip(&remote,ip,256);
                MY_PRINTF("remote url %s:%d\n", ip, remote.port);  
       //         MY_PRINTF("connect remote stream server ok\n");                                
        }

        else
        {
                MY_PRINTF("connect remote stream server fail\n");                
                return;
        }

//        while (1)
//        {
                char send_buf[PACKET_LEN]={0};
                enum server_request_t request = FROM_PC; 
                enum channel_type_t channel = CONTROL_CHANNEL;                 

                memcpy(send_buf,  &request, 4);
                strcpy(send_buf + 4, "jason");
                memcpy(send_buf + 12,  &channel, 4);

                packet=enet_packet_create(NULL, packet_len, ENET_PACKET_FLAG_RELIABLE);                 /* 创建包 */
                memcpy(packet->data, send_buf, packet_len);
                if (enet_peer_send(center_peer, 1, packet) < 0)
                        MY_PRINTF("enet_peer_send fail\n");                
                printf("start send data");

#if 0
                packet=enet_packet_create(NULL, packet_len, ENET_PACKET_FLAG_RELIABLE);                 /* 创建包 */
                strcpy((char*)packet->data, "send control punching packet");
                if (enet_peer_send(control_center_peer, 1, packet) < 0)
                MY_PRINTF("enet_peer_send fail\n");                
                enet_host_service(control_host, &event, 1000);
#endif                

                if (enet_host_service(login_host, &event, 200)>=0)
                {
                        if(event.type == ENET_EVENT_TYPE_CONNECT) //有客户机连接成功
                        {
                        /*
                                if (event.peer == NULL)
                                        printf("connect server event.peer is NULL\n");
                                
                                printf("success to connect server\n");
                                */
                                char ip[256];    
                                ENetAddress remote = event.peer->address; //远程地址
                                
                                enet_address_get_host_ip(&remote,ip,256);
                                printf("remote url %s:%d\n", ip, remote.port);                                
                        }
                        else if(event.type==ENET_EVENT_TYPE_RECEIVE) //收到数据
                        {
                                int type;
                                ENetAddress remote = event.peer->address; //远程地址

                                printf("recv data from center server\n");

                                memcpy(ipcam_url.ip, event.packet->data, 32);
                                memcpy(&ipcam_url.port, event.packet->data + 32, 4); 
                                memcpy(&type, event.packet->data + 36, 4);
                                MY_PRINTF("ipcam info:\tip:%s, port:%d, type:%s\n", ipcam_url.ip, ipcam_url.port, 
                                                                                                        type ? "STREAM_TYPE" : "CONTROL_TYPE");
                                enet_packet_destroy(event.packet);    //注意释放空间
#if 1                 
                                /* 创建线程连接IPCAM */
                                if (type == CONTROL_CHANNEL) 
                                        pthread_create(&control_id, NULL, control_thread, &ipcam_url);
                                else
                                        pthread_create(&stream_id, NULL, stream_thread, &ipcam_url);                                
#endif                                
                        }
                        else if(event.type==ENET_EVENT_TYPE_DISCONNECT) //失去连接
                        {
                                printf("connect is down port %d\n", event.peer->address.port);
                        }
                }
                
   //             sleep(1); 
  //      }

        while(1) sleep(1);
}
