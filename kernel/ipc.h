#ifndef IPC_H
#define IPC_H

#include "../lib/types.h"

#define IPC_MAX_PORTS       256
#define IPC_MAX_MESSAGES    64
#define IPC_MSG_SIZE        128
#define IPC_PORT_INVALID    0xFFFFFFFF

#define IPC_MSG_NONE        0x00
#define IPC_MSG_DATA        0x01
#define IPC_MSG_SIGNAL      0x02
#define IPC_MSG_REQUEST     0x03
#define IPC_MSG_RESPONSE    0x04
#define IPC_MSG_ERROR       0x05

#define IPC_ERR_NONE        0
#define IPC_ERR_FULL        1
#define IPC_ERR_EMPTY       2
#define IPC_ERR_INVALID     3
#define IPC_ERR_PERM        4

typedef struct {
    uint32_t sender;
    uint32_t receiver;
    uint32_t type;
    uint32_t length;
    uint8_t  data[IPC_MSG_SIZE];
    uint64_t timestamp;
} PACKED ipc_message_t;

typedef struct {
    uint32_t      port_id;
    uint32_t      owner_pid;
    uint32_t      head;
    uint32_t      tail;
    uint32_t      count;
    bool          active;
    ipc_message_t messages[IPC_MAX_MESSAGES];
} ipc_port_t;

void     ipc_init();
uint32_t ipc_create_port(uint32_t owner_pid);
int      ipc_destroy_port(uint32_t port_id);
int      ipc_send(uint32_t port_id, uint32_t sender, uint32_t type, void *data, uint32_t len);
int      ipc_receive(uint32_t port_id, ipc_message_t *msg);
bool     ipc_port_has_messages(uint32_t port_id);
uint32_t ipc_get_message_count(uint32_t port_id);
void     ipc_dump_port(uint32_t port_id);

#endif
