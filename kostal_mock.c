#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <modbus/modbus.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "kostal_modbus.h"

modbus_t *ctx;
modbus_mapping_t *mb_mapping;
int server_socket = -1;

void close_server(int signum) {
    printf("Shutting down Kostal Modbus TCP Server...\n");
    if (server_socket != -1) close(server_socket);
    if (mb_mapping) modbus_mapping_free(mb_mapping);
    if (ctx) {
        modbus_close(ctx);
        modbus_free(ctx);
    }
    exit(0);
}

// Convert float to Modbus register format (ABCD layout)
void float_to_registers(float value, uint16_t *base, int offset) {
    if (offset < 0 || offset + 1 >= NUM_REGISTERS) {
        fprintf(stderr, "Register write out of bounds at %d\n", offset);
        exit(1);
    }
    uint16_t *dest = &base[offset];
    uint32_t as_int;
    memcpy(&as_int, &value, sizeof(float));

    dest[0] = (as_int >> 16) & 0xFFFF;  // High word
    dest[1] = as_int & 0xFFFF;         // Low word
}

void init(void) {
    signal(SIGINT, close_server);

    ctx = modbus_new_tcp(NULL, PORT);
    if (ctx == NULL) {
        fprintf(stderr, "Unable to create Modbus context\n");
        exit(1);
    }

    mb_mapping = modbus_mapping_new(0, 0, NUM_REGISTERS, 0);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate Modbus mapping: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        exit(1);
    }

    server_socket = modbus_tcp_listen(ctx, 1);
    if (server_socket == -1) {
        perror("Modbus listen failed");
        close_server(0);
    }

    modbus_tcp_accept(ctx, &server_socket);
    printf("Kostal Modbus TCP Server started on port %d\n", PORT);

}

int main(void) {
    init();


    for (;;) {
        // values not changing now, could be made random: ((float)rand()/(float)(RAND_MAX)) * max_val
        float_to_registers(50.0f,  mb_mapping->tab_registers, KOSTAL_MODBUS_REG_OWN_CONSUMPTION_BATTERY);
        float_to_registers(100.0f, mb_mapping->tab_registers, KOSTAL_MODBUS_REG_OWN_CONSUMPTION_GRID);
        float_to_registers(800.0f, mb_mapping->tab_registers, KOSTAL_MODBUS_REG_OWN_CONSUMPTION_PV);
        float_to_registers(60.0f, mb_mapping->tab_registers, KOSTAL_MODBUS_REG_BATTERY_TEMP);
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        int rc = modbus_receive(ctx, query);
        if (rc == -1) {
            break;
        }
        modbus_reply(ctx, query, rc, mb_mapping);
    }

    close_server(0);
    return 0;
}
