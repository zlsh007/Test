#ifndef __HEAR_H__
#define __HEAR_H__

static int debug = 0;
#define HEAR(fmt,args...)  do{	\
	if(debug!=0){				\
		printf("[%s-%d-%s]: ",__FILE__,__LINE__,__FUNCTION__);	\
		printf(fmt,##args);		\
	}						\
}while(0)

#endif
