# Virtual Memory Simulator “vmsim” in C

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
