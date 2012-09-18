/******************************************************************************/
/*                                                                            */
/******************************************************************************/

#ifndef netif_h__
#define netif_h__


#include "list.h"


typedef struct interface_s
{
    char *name;
    long recv;
    long sent;
} *interface_t;


char * get_if(char *buffer);
long get_recv(const char *if_name);
long get_sent(const char *if_name);
interface_t grab_if(char *buffer);
list_t iflist(char *buffer);


#endif // netif_h__

