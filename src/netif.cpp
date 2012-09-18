/******************************************************************************/
/*                                                                            */
/******************************************************************************/

#include <cstdlib>
#include <cstring>

#include "netif.h"
#include "strutil.h"


/******************************************************************************/
/* Return interface name mentioned in buffer                                                                           */
/******************************************************************************/
char * get_if(char *buffer)
{
    char *if_name = strchr(buffer, ':');

    if (if_name == 0)
    {
        return 0;
    }

    *(if_name + 1) = 0;
    while (!is_delim(if_name))
    {
        if_name--;
    }

    return if_name + 1;
}

/******************************************************************************/
/* Return received bytes for specified interface                                                                           */
/******************************************************************************/
long get_recv(const char *if_name)
{
    const char *if_recv = strchr(if_name, ':') + 2;

    if_recv = skip_delim(if_recv);

    return atoll(if_recv);
}

/******************************************************************************/
/* Return sent bytes for specified interface                                                                           */
/******************************************************************************/
long get_sent(const char *if_name)
{
    const char *if_sent = strchr(if_name, ':') + 2;

    for (int skip = 0; skip < 8; ++skip)
    {
        if_sent = skip_delim(if_sent);
        if_sent = skip_word(if_sent);
    }

    if_sent = skip_delim(if_sent);

    return atoll(if_sent);
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
interface_t grab_if(char *buffer)
{
    if (char *name = get_if(buffer))
    {
        interface_t pif = (interface_t) malloc(sizeof(interface_s));

        pif->name = name;
        pif->recv = get_recv(pif->name);
        pif->sent = get_sent(pif->name);

        return pif;
    }

    return 0;
}

/******************************************************************************/
/*                                                                            */
/******************************************************************************/
list_t iflist(char *buffer)
{
    list_t list = list_init();
    while(interface_t pif = grab_if(buffer))
    {
        list_add(list, pif);
        buffer = buffer + strlen(buffer) + 1;
    }

    return list;
}

