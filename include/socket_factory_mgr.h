#ifndef ASHTI_SRC_SOCKET_FACTORY_SOCKET_FACTORY_MGR_H_
#define ASHTI_SRC_SOCKET_FACTORY_SOCKET_FACTORY_MGR_H_
#include <socket_factory.h>

void clear_char(char * buffer, size_t buffer_sz);
char * get_ip(struct sockaddr_storage * config);
char * get_port(struct sockaddr_storage * config);
socket_factory_t *factory_init(size_t payload_size);
void clear_buffer(socket_factory_t * factory);
void reset_storage(socket_factory_t * factory);
void free_source_client(socket_factory_t * factory);
void populate_source_info(socket_factory_t *factory);
char * get_full_path(const char * path);

#endif //ASHTI_SRC_SOCKET_FACTORY_SOCKET_FACTORY_MGR_H_
