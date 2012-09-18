/******************************************************************************/
/*                                                                            */
/******************************************************************************/

#ifndef list_h__
#define list_h__


typedef struct node_s
{
    void *data;
    node_s *next;
} *node_t;

typedef struct list_s
{
    node_t head;
    node_t tail;
} *list_t;


list_t list_init();
node_t list_add(list_t list, void *data);
void list_clean(list_t list);
void list_destroy(list_t list);


#endif // list_h__

