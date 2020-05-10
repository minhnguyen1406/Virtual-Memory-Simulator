# Virtual Memory Simulator “vmsim” in C

A program that translates logical to physical addresses for a virtual address space of size 2<sup>16</sup> = 65536 bytes. It will read from a file containing logical addresses using a TLB as well as a page table, will translate each logical address to its corresponding physical address and output the value of the byte stored at the translated physical address.

### SYNOPSIS:
To compile this program:
```
$ make
```
To run this program:
```
$ ./vmsim -s <swap_file> -a <address_file> -m <mode>
```
To remove the executable:
```
$ make clean
```

### OPTIONS:
*	**−s** : followed by the name of the swap file or backing store file used for this simulation
*	**−a** : followed by the name of the address file (list of addresses)
*	**−m** : followed by the mode
	*	**DEMAND** (Demand paging)
    *	**FIFO** (First-in-first-out)
    *	**LRU** (Least-recently-used stack implementation)

### OUTPUTS:
The output will printed out as standard output on terminal. The expected output for each mode will be in the correct_<mode>.txt files.
