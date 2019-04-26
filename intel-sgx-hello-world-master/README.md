# Intel SGX "Hello World"

This project is based on [digawp/hello-enclave](https://github.com/digawp/hello-enclave) sample code and sample projects provided with the Intel SGX Linux  [SDK](https://github.com/01org/linux-sgx/).

### Description

**The application cumulatively adds user provided numbers inside a secure environment, then seales the result and stores it in the filesystem. Sealing and unsealing is done inside the enclave, so no one can tamper with the stored value nor the code that operates on it.**

The flow is as follows:
 - untrusted code loads the encrypted 'sealed.data' file, then invokes enclave code
 - enclave unseals the data (extracting an integer number), executes trused code (adding the user-provided number and the previously stored number) then seals back the data
 - untrused code stores the encrypted again data into 'sealed.data' file

Messages printing the content of the sealed data is included only to see how the app works.

### Building

Install Intel SGX Linux driver and SDK then just run:
```
$ make
```
By default the application is run in simulation mode. If you want to compile it with real hardware support, run the build with:
```
$ make SGX_MODE=HW
```
### Running

```
$ ./app
Provide a number to add: 10
"sealed.data" file not found. Creating new one...
Initializing enclave data with value: 0
Printing from enclave, just to see what's inside: 10

$ ./app
Provide a number to add: 5
Printing from enclave, just to see what's inside: 15

$ cat sealed.data
H �7j���M;zKH�x
.VvdPD�����,2�<���M-��j~�C�z���Ԟ%�%
```

### LICENSE

This project is licensed under the **MIT license**.
