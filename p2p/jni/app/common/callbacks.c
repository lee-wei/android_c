/** 
 @file callbacks.c
 @brief ENet callback functions
*/
#define ENET_BUILDING_LIB 1
#include "enet/enet.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

static ENetCallbacks callbacks = { malloc, free, rand };

static unsigned long mem_counter = 0;

int
enet_initialize_with_callbacks (ENetVersion version, const ENetCallbacks * inits)
{
   if (version != ENET_VERSION)
     return -1;

   if (inits -> malloc != NULL || inits -> free != NULL)
   {
      if (inits -> malloc == NULL || inits -> free == NULL)
        return -1;

      callbacks.malloc = inits -> malloc;
      callbacks.free = inits -> free;
   }
      
   if (inits -> rand != NULL)
     callbacks.rand = inits -> rand;

   return enet_initialize ();
}
           
void *
enet_malloc (size_t size)
{
   void * memory = callbacks.malloc (size);

   //if (memory == NULL)
   //  abort ();
     
   //printf("enet_malloc %ld,%d,%d\n", (unsigned long)memory, size, ++mem_counter);

   return memory;
}

void
enet_free (void * memory)
{
   callbacks.free (memory);
   //printf("enet_free %ld, %d\n", (unsigned long)memory, --mem_counter);
}

int
enet_rand (void)
{
   return callbacks.rand ();
}

