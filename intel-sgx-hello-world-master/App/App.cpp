#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include "Enclave_u.h"
#include "sgx_urts.h"
//#include "sgx_utils/sgx_utils.h"
#include "sgx_utils.cpp"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;
const char *data_filename = "sealed.data";

// OCALL implementations
void print_message(const char* str) {
    std::cout << str << std::endl;
}



int read_number() {
  int input;
  std::cout << "Provide a number to add: ";
  std::cin >> input;
  return std::cin.fail() ? -1 : input;
}

int load_data(uint8_t *sealed_data, size_t sealed_size) {
  FILE *fp = fopen(data_filename, "rb");
  if (fp == NULL) {
      printf("\"%s\" file not found. Creating new one...\n", data_filename);
      initialize_enclave_data(global_eid, sealed_data, sealed_size, 0);
  } else {
      size_t read_num = fread(sealed_data, 1, sealed_size, fp);
      if (read_num != sealed_size) {
          printf("Warning: Failed to read sealed data from \"%s\" (%zu bytes read).\n", data_filename, read_num);
          return -1;
      }
      fclose(fp);
  }
}

int save_data(uint8_t *sealed_data, size_t sealed_size) {
  FILE *fp = fopen("sealed.data", "wb");
  if (fp) {
      size_t write_num = fwrite(sealed_data, 1, sealed_size, fp);
      if (write_num != sealed_size) {
          printf("Warning: Failed to save sealed data to \"%s\" (%zu bytes written).\n", data_filename, write_num);
          return -1;
        }
      fclose(fp);
  }
}

int main(int argc, char const *argv[]) {
    int input;
    if ((input = read_number()) < 0) {
      std::cout << "Provide a correct integer number!" << std::endl;
      return -1;
    }

    if (initialize_enclave(&global_eid, "enclave.token", "enclave.signed.so") < 0) {
        std::cout << "Fail to initialize enclave." << std::endl;
        return 1;
    }

    size_t sealed_size = sizeof(sgx_sealed_data_t) + sizeof(int);
    uint8_t* sealed_data = (uint8_t*) malloc(sealed_size);

    if (load_data(sealed_data, sealed_size) < 0) {
      return -1;
    };
    
    execute_secure_operation(global_eid, sealed_data, sealed_size, input);

    if (save_data(sealed_data, sealed_size) < 0) {
      return -1;
    }

    return 0;
}
