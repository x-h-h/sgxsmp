#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include "Enclave_u.h"
#include "sgx_urts.h"
#include "sgx_utils/sgx_utils.h"

/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;
const char *data_filename = "sealed.data";

// OCALL implementations
void print_message(const char* str) {
    std::cout << str << std::endl;
}

int initialize_enclave(sgx_enclave_id_t *eid, const std::string &launch_token_path, const std::string &enclave_name)
{
  const char *token_path = launch_token_path.c_str();
  sgx_launch_token_t token = {0};
  sgx_status_t ret = SGX_ERROR_UNEXPECTED;
  int updated = 0;

  /* Step 1: try to retrieve the launch token saved by last transaction
     *         if there is no token, then create a new one.
     */
  FILE *fp = fopen(token_path, "rb");
  if (fp == NULL && (fp = fopen(token_path, "wb")) == NULL)
  {
    printf("Warning: Failed to create/open the launch token file \"%s\".\n", token_path);
  }

  if (fp != NULL)
  {
    /* read the token from saved file */
    size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);
    if (read_num != 0 && read_num != sizeof(sgx_launch_token_t))
    {
      /* if token is invalid, clear the buffer */
      memset(token, 0x0, sizeof(sgx_launch_token_t));
      printf("Warning: Invalid launch token read from \"%s\".\n", token_path);
    }
  }
  /* Step 2: call sgx_create_enclave to initialize an enclave instance */
  /* Debug Support: set 2nd parameter to 1 */
  ret = sgx_create_enclave(enclave_name.c_str(), SGX_DEBUG_FLAG, &token, &updated, eid, NULL);
  if (ret != SGX_SUCCESS)
  {
    print_error_message(ret);
    if (fp != NULL)
      fclose(fp);
    return -1;
  }

  /* Step 3: save the launch token if it is updated */
  if (updated == 0 || fp == NULL)
  {
    /* if the token is not updated, or file handler is invalid, do not perform saving */
    if (fp != NULL)
      fclose(fp);
    return 0;
  }

  /* reopen the file with write capablity */
  fp = freopen(token_path, "wb", fp);
  if (fp == NULL)
    return 0;
  size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);
  if (write_num != sizeof(sgx_launch_token_t))
    printf("Warning: Failed to save launch token to \"%s\".\n", token_path);
  fclose(fp);
  return 0;
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
