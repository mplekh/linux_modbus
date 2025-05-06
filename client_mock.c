#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <modbus/modbus.h>
#include <stdint.h>
#include <string.h>

#include "kostal_modbus.h"

// Convert two Modbus registers to float (ABCD format)
void read_float(modbus_t *ctx, int addr, const char *label) {
    uint16_t regs[2];
    int rc = modbus_read_registers(ctx, addr, 2, regs);
    if (rc != 2) {
        fprintf(stderr, "Failed to read %s at 0x%X: %s\n", label, addr, modbus_strerror(errno));
        return;
    }
    uint32_t as_int = ((uint32_t)regs[0] << 16) | regs[1];

    float val;
    memcpy(&val, &as_int, sizeof(float));
    printf("%s (0x%X): %.2f\n", label, addr, val);
}

int main(void) {
    modbus_t *ctx = modbus_new_tcp("127.0.0.1", PORT);  // Connect to local server
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return 1;
    }
    for (int i = 0; i < 3; ++i) {
        printf("Read registers from mock (iteration %d):\n", i);
        read_float(ctx, KOSTAL_MODBUS_REG_OWN_CONSUMPTION_BATTERY,  "Battery consumption");
        read_float(ctx, KOSTAL_MODBUS_REG_OWN_CONSUMPTION_GRID,  "Grid power");
        read_float(ctx, KOSTAL_MODBUS_REG_OWN_CONSUMPTION_PV, "PV power");
        read_float(ctx, KOSTAL_MODBUS_REG_BATTERY_TEMP, "Battery temperature");
        printf("\n");
    }
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}
