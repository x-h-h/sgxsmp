#include <cstdio>
#include "Enclave_t.h"

char message[200];

void execute_secure_operation(uint8_t* sealed_data, uint32_t sealed_data_size, int value) {
    int unsealed_value;
    sgx_status_t status = unseal((sgx_sealed_data_t*) sealed_data, sealed_data_size,
            (uint8_t*) &unsealed_value, sizeof(unsealed_value));

    unsealed_value += value;

    seal((uint8_t*) &unsealed_value, sizeof(unsealed_value),
            (sgx_sealed_data_t*) sealed_data, sealed_data_size);

    /* This data should never leave enclave, here it's just for the demo. */
    snprintf(message, 200, "Printing from enclave, just to see what's inside: %d", unsealed_value);
    print_message(message);
}

void initialize_enclave_data(uint8_t* sealed_data, uint32_t sealed_data_size, int initial_value) {
    seal((uint8_t*) &initial_value, sizeof(initial_value),
            (sgx_sealed_data_t*) sealed_data, sealed_data_size);

    /* This data should never leave enclave, here it's just for the demo. */
    snprintf(message, 200, "Initializing enclave data with value: %d", initial_value);
    print_message(message);
}
