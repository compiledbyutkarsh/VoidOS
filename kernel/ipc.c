#include "ipc.h"
#include "../drivers/vga.h"

static ipc_port_t ipc_ports[IPC_MAX_PORTS];
static uint32_t   ipc_port_count = 0;

static uint64_t ipc_get_timestamp() {
    uint64_t tsc;
    __asm__ volatile("rdtsc" : "=A"(tsc));
    return tsc;
}

void ipc_init() {
    for (int i = 0; i < IPC_MAX_PORTS; i++) {
        ipc_ports[i].port_id   = IPC_PORT_INVALID;
        ipc_ports[i].owner_pid = 0;
        ipc_ports[i].head      = 0;
        ipc_ports[i].tail      = 0;
        ipc_ports[i].count     = 0;
        ipc_ports[i].active    = false;
    }
    ipc_port_count = 0;
    vga_puts("[IPC]  Port table initialized | Max ports: 256\n");
}

uint32_t ipc_create_port(uint32_t owner_pid) {
    for (int i = 0; i < IPC_MAX_PORTS; i++) {
        if (!ipc_ports[i].active) {
            ipc_ports[i].port_id   = i;
            ipc_ports[i].owner_pid = owner_pid;
            ipc_ports[i].head      = 0;
            ipc_ports[i].tail      = 0;
            ipc_ports[i].count     = 0;
            ipc_ports[i].active    = true;
            ipc_port_count++;
            return i;
        }
    }
    return IPC_PORT_INVALID;
}

int ipc_destroy_port(uint32_t port_id) {
    if (port_id >= IPC_MAX_PORTS) return IPC_ERR_INVALID;
    if (!ipc_ports[port_id].active) return IPC_ERR_INVALID;

    ipc_ports[port_id].active    = false;
    ipc_ports[port_id].port_id   = IPC_PORT_INVALID;
    ipc_ports[port_id].head      = 0;
    ipc_ports[port_id].tail      = 0;
    ipc_ports[port_id].count     = 0;
    ipc_port_count--;

    return IPC_ERR_NONE;
}

int ipc_send(uint32_t port_id, uint32_t sender, uint32_t type, void *data, uint32_t len) {
    if (port_id >= IPC_MAX_PORTS)         return IPC_ERR_INVALID;
    if (!ipc_ports[port_id].active)       return IPC_ERR_INVALID;
    if (ipc_ports[port_id].count >= IPC_MAX_MESSAGES) return IPC_ERR_FULL;

    ipc_port_t    *port = &ipc_ports[port_id];
    ipc_message_t *msg  = &port->messages[port->tail];

    msg->sender    = sender;
    msg->receiver  = port->owner_pid;
    msg->type      = type;
    msg->timestamp = ipc_get_timestamp();

    if (len > IPC_MSG_SIZE) len = IPC_MSG_SIZE;
    msg->length = len;

    if (data && len > 0) {
        uint8_t *src = (uint8_t*)data;
        for (uint32_t i = 0; i < len; i++) {
            msg->data[i] = src[i];
        }
    }

    port->tail  = (port->tail + 1) % IPC_MAX_MESSAGES;
    port->count++;

    return IPC_ERR_NONE;
}

int ipc_receive(uint32_t port_id, ipc_message_t *out) {
    if (port_id >= IPC_MAX_PORTS)   return IPC_ERR_INVALID;
    if (!ipc_ports[port_id].active) return IPC_ERR_INVALID;
    if (ipc_ports[port_id].count == 0) return IPC_ERR_EMPTY;

    ipc_port_t    *port = &ipc_ports[port_id];
    ipc_message_t *msg  = &port->messages[port->head];

    if (out) {
        out->sender    = msg->sender;
        out->receiver  = msg->receiver;
        out->type      = msg->type;
        out->length    = msg->length;
        out->timestamp = msg->timestamp;
        for (uint32_t i = 0; i < msg->length; i++) {
            out->data[i] = msg->data[i];
        }
    }

    port->head  = (port->head + 1) % IPC_MAX_MESSAGES;
    port->count--;

    return IPC_ERR_NONE;
}

bool ipc_port_has_messages(uint32_t port_id) {
    if (port_id >= IPC_MAX_PORTS)   return false;
    if (!ipc_ports[port_id].active) return false;
    return ipc_ports[port_id].count > 0;
}

uint32_t ipc_get_message_count(uint32_t port_id) {
    if (port_id >= IPC_MAX_PORTS)   return 0;
    if (!ipc_ports[port_id].active) return 0;
    return ipc_ports[port_id].count;
}

void ipc_dump_port(uint32_t port_id) {
    if (port_id >= IPC_MAX_PORTS) return;
    ipc_port_t *port = &ipc_ports[port_id];
    vga_printf("[IPC]  Port %d | Owner: %d | Messages: %d | Active: %d\n",
        port->port_id, port->owner_pid, port->count, port->active
    );
}
