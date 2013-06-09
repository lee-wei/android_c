#ifndef		__DEBUG_H__
#define		__DEBUG_H__

#include <stdio.h>

#define		DEBUG		0

#define		MY_PRINTF(args...)				\
do{								\
	if (DEBUG) 						\
	{							\
		printf("%s==%d:\t", __FILE__, __LINE__);	\
		printf(args);					\
	}							\
}while(0)							

#endif
