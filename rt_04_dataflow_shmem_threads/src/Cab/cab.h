#ifndef CAB
#define CAB

// #include "buffer.h"

typedef struct {
    char * name;
    int dim;
    int num;
    int * array;
    u_int8_t * img_base;
    u_int8_t * img_last;

} cab;

cab * open_cab(char * name,int num,int dim,u_int8_t * first);

 u_int8_t *  reserve(cab * cab_id);

void put_mes( u_int8_t * buf_pointer,cab * cab_id);

 u_int8_t *  get_mes(cab * cab_id);

void unget( u_int8_t *  mes_pointer, cab * cab_id);

#endif
