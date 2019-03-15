# Welcome to the Benchmarking tool

The benchmarking tool is intended for embedded software benchmarking with 
low/none overhead intrusion. The tool (will) be able to benchmark:

- Performance execution using PC sampling a regular frequency (1/16384 CPU cycle).
- Memory:
  - Static analysis: (To be implemented) Information of the text + bss usage.
  - Stack analysis per thread: (To be implemented) Retrieve per thread, the amount 
				of stack. 
  - Heap analysis per thread: (To be implemented) Retrieve per thread, the amount 
				of dynamic allocation. 


The performance execution benchmarking is intended to work with any applications
running on a microcontroller with the ARM-coresight debug system implemented.

However the *Stack* and *Heap* memory analysis will first target the nuttx *kernel*. 

## Software Prerequisites
The application was tested on Ubuntu 18.04.1 LTS. It shall work on other

The list of packet needed for the application to work:

- autoconf
- autotools-dev
- binutils
- check
- doxygen 
- gcc-arm-none-eabi
- git
- libtool
- libusb-1.0
- libftdi-dev
- make
- pkg-config
- texinfo

## Hardware Prerequisites 
The benchmarking tool is using hardware debugger to reduce the overhead in code.
Status  about the different debugger/serial/board that t

### Debuggers
List of debug probe supported:

| Debugger Probe     | Status     |
| -----------------  | ---------- |
| [ST Link v2.1](http://www.st.com/en/development-tools/st-link-v2.html)       | Working    |

### UART (SWO output)
Every UART device that support the format 115200 8N1 should be working.

### Board
List of boards:

| Board              | Status execution | Status memory |
| -----------------  | ---------------- | ------------- |
| [Olimex E407](https://www.olimex.com/Products/ARM/ST/STM32-E407/open-source-hardware) | Working | Not available |
| [STM32L152-Discovery](https://www.st.com/en/evaluation-tools/32l152cdiscovery.html) | Not Tested | Not available |

## Compiling 
To compile:
```console
foo@bar:~$ ./autogen.sh # Will retrieve dependencies and compile them.
foo@bar:~$ ./configure
foo@bar:~$ make
```	
### Configuration file
Before execution the configuration file need to be changed/adapted depending on
the use. The configuration file used by the application is located at 
__res/tests/test_config.ini__

More explanations about the fields are available in the template located at
__res/configs/default_decoder.ini__



## Execution
The compiled application will be located at __apps/decoder__. 

To execute it:

```console
foo@bar:~$ ./apps/decoder
```

Before executing, the configuration file shall be filled appropriately and
UART and SWD debugger shall be connected to the targeted embedded platform.



## TODOS
- [ ] Use command line arguments to pass configuration file.
- [ ] Memory benchmarking 
    - [ ] Static memory analysis.
    - [ ] Stack memory usage per threads.
    - [ ] Heap memory usage per threads.
- [ ] Integrate it in the CI.
- [ ] Dynamic log level.

