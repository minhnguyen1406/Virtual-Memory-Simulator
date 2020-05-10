//Minh Tuan Nguyen
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MASK 0x00FF
#define PAGE_SHIFT 8
#define BUFFER_SIZE 10
#define TLB_SIZE 16
#define NUM_FRAMES 256
#define NUM_FRAMES_MOD 128
#define FRAME_SIZE 256
#define NUM_PAGES 256
#define NUM_PAGES_MOD 128
#define PAGE_SIZE 256

//structs
typedef struct
{
  int page_number;
  int frame_number;
} page_frame_node;

//function prototypes
void init_memory();

void init_memory_mod();

void init_page_table();
int check_page_table_demand(int page_number);
void insert_to_page_table_demand(int page_number, int frame_number);

void init_page_table_mod();

int check_page_table_fifo(int page_number);
void insert_to_page_table_fifo(int page_number, int frame_number);

int check_page_table_lru(int page_number);
int insert_to_page_table_lru(int page_number, int frame_number);
void shift_page_table_lru(int index);

void init_TLB();
int check_TLB(int page_number);
void insert_to_TLB(int page_number, int frame_number);

void demand(char *address, FILE *swap_file, FILE *address_file);
void fifo(char *address, FILE *swap_file, FILE *address_file);
void lru(char *address, FILE *swap_file, FILE *address_file);

//global variables

//files
FILE *address_file;
FILE *swap_file;

//arrays
signed char physical_memory[NUM_FRAMES][FRAME_SIZE];
int page_table[NUM_PAGES];
page_frame_node TLB[TLB_SIZE];

signed char physical_memory_mod[NUM_FRAMES_MOD][FRAME_SIZE];
page_frame_node page_table_mod[NUM_PAGES_MOD];

//reading variables
char address[BUFFER_SIZE];
int logical_address;
signed char read_value;

//counts
int address_count = 0;
int frame_count = 0;
int faults_count = 0;
int TLB_hits = 0;
int TLB_count = 0;
char *input_swap_file;
char *input_address_file;
int mode = 0; //1 : DEMAND, 2: FIFO, 3: LRU

int main(int argc, char *argv[])
{
  //handle number of arguments
  if (argc != 7)
  {
    fprintf(stdout, "Usage: ./vmsim -s <swap_file> -a <address_file> -m <mode (i.e: DEMAND or FIFO or LRU)>\n");
    return -1;
  }
  for (int i = 0; i < argc; i++)
  {
    if (strcmp(argv[i], "-s") == 0)
    {
      //swap file name
      input_swap_file = argv[i + 1];
    }
    if (strcmp(argv[i], "-a") == 0)
    {
      //address file name
      input_address_file = argv[i + 1];
    }
    if (strcmp(argv[i], "-m") == 0)
    {
      if (strcmp(argv[i + 1], "DEMAND") == 0)
      {
        //mode = 1 : DEMAND
        mode = 1;
      }
      else if (strcmp(argv[i + 1], "FIFO") == 0)
      {
        //mode = 2 : FIFO
        mode = 2;
      }
      else if (strcmp(argv[i + 1], "LRU") == 0)
      {
        //mode = 3 : DEMAND
        mode = 3;
      }
      else
      {
        //handle syntax error
        fprintf(stdout, "Usage: DEMAND or FIFO or LRU\n");
        return -1;
      }
    }
  }

  //open swap file
  swap_file = fopen(input_swap_file, "rb");

  //handle null swap file
  if (swap_file == NULL)
  {
    fprintf(stderr, "Error opening %s\n", input_swap_file);
    return -1;
  }

  //open address file
  address_file = fopen(input_address_file, "r");

  //handle null address file
  if (address_file == NULL)
  {
    fprintf(stderr, "Error opening %s\n", input_address_file);
    return -1;
  }

  //if DEMAND, initialize memory and page table 
  if (mode == 1)
  {
    init_memory();
    init_page_table();
  }

  //if FIFO or LRU, initialize memory modifed and page table  modifed (half the size of swap file)
  else
  {
    init_memory_mod();
    init_page_table_mod();
  }
  //initialize TLB regardless of the mode
  init_TLB();

  if (mode == 1)
  {
    //DEMAND algorithm
    demand(address, swap_file, address_file);
  }
  else if (mode == 2)
  {
    //FIFO algorithm
    fifo(address, swap_file, address_file);
  }
  else
  {
    //LRU algorithm
    lru(address, swap_file, address_file);
  }

  //print statistics
  printf("Number of Translated Addresses = %d\n", address_count);
  printf("Page Faults = %d\n", faults_count);
  printf("Page Fault Rate = %0.3f\n", (double)faults_count / (double)address_count);
  printf("TLB Hits = %d\n", TLB_hits);
  printf("TLB Hit Rate = %0.3f\n", (double)TLB_hits / (double)address_count);

  //close opened files
  fclose(address_file);
  fclose(swap_file);

  return 0;
}

//initialize memory
void init_memory()
{
  for (int i = 0; i < NUM_FRAMES; i++)
  {
    for (int j = 0; j < FRAME_SIZE; j++)
      physical_memory[i][j] = 0;
  }
}

//initialize memory modifed 
void init_memory_mod()
{
  for (int i = 0; i < NUM_FRAMES_MOD; i++)
  {
    for (int j = 0; j < FRAME_SIZE; j++)
      physical_memory_mod[i][j] = 0;
  }
}

//initialize page table 
void init_page_table()
{
  for (int i = 0; i < NUM_PAGES; i++)
  {
    page_table[i] = -1;
  }
}

//retrieve frame number at a given page number
int check_page_table_demand(int page_number)
{
  return page_table[page_number];
}

//insert a mapped node to the page table
void insert_to_page_table_demand(int page_number, int frame_number)
{
  page_table[page_number] = frame_number;
}

//initialize page talbe modified
void init_page_table_mod()
{
  for (int i = 0; i < NUM_PAGES_MOD; i++)
  {
    page_table_mod[i].page_number = -1;
    page_table_mod[i].frame_number = -1;
  }
}

//retrieve frame number at a given page number; returns -1 if it is not found
int check_page_table_fifo(int page_number)
{
  for (int i = 0; i < NUM_PAGES_MOD; i++)
  {
    if (page_table_mod[i].page_number == page_number)
      return page_table_mod[i].frame_number;
  }

  return -1;
}

//insert a mapped node to the page table 
void insert_to_page_table_fifo(int page_number, int frame_number)
{
  page_table_mod[frame_number].page_number = page_number;
  page_table_mod[frame_number].frame_number = frame_number;
}

//retrieve frame number at a given page number; returns -1 if it is not found
int check_page_table_lru(int page_number)
{

  for (int i = 0; i < NUM_PAGES_MOD; i++)
  {
    if (page_table_mod[i].page_number == page_number)
      return i;
  }

  return -1;
}

//insert a mapped node to the page table 
int insert_to_page_table_lru(int page_number, int frame_number)
{
  for (int i = NUM_PAGES_MOD - 1; i > 0; i--)
    page_table_mod[i] = page_table_mod[i - 1];

  page_table_mod[0].page_number = page_number;
  page_table_mod[0].frame_number = frame_count;

  return frame_count;
}

//shift page table from index to the left
void shift_page_table_lru(int index)
{
  page_frame_node front = page_table_mod[index];

  for (int i = index; i > 0; i--)
    page_table_mod[i] = page_table_mod[i - 1];

  page_table_mod[0] = front;
}

//initialize TLB
void init_TLB()
{
  for (int i = 0; i < TLB_SIZE; i++)
  {
    TLB[i].page_number = -1;
    TLB[i].frame_number = -1;
  }
}

//check if a particular page number is in TLB
int check_TLB(int page_number)
{
  for (int i = 0; i < TLB_SIZE; i++)
  {
    if (TLB[i].page_number == page_number)
      return TLB[i].frame_number;
  }

  return -1;
}

//insert a mapped node to TLB
void insert_to_TLB(int page_number, int frame_number)
{
  TLB[TLB_count].page_number = page_number;
  TLB[TLB_count].frame_number = frame_number;

  TLB_count = (TLB_count + 1) % TLB_SIZE;
}

//DEMAND algorithm
void demand(char *address, FILE *swap_file, FILE *address_file)
{
  //read each number from address file
  while (fgets(address, BUFFER_SIZE, address_file) != 0)
  {
    //increment number of addresses count
    address_count++;

    //convert address read to integer
    logical_address = atoi(address);

    //bit-masking and bit-shifting to retrieve the page number and offset from the logical address
    int page_number = logical_address >> PAGE_SHIFT;
    page_number = page_number & MASK;
    int offset = logical_address & MASK;

    //check the TLB for the page number
    int frame_number = check_TLB(page_number);

    //if it is not in the TLB
    if (frame_number == -1)
    {
      //check the page table for the page number
      frame_number = check_page_table_demand(page_number);

      //if it is not in the page table
      if (frame_number == -1)
      {
        //create frame number from frame count
        frame_number = frame_count++ % NUM_PAGES;
        //add a mapped node to page table
        insert_to_page_table_demand(page_number, frame_number);

        //increament faults count
        faults_count++;

        //seek to location in swap file
        if (fseek(swap_file, page_number * PAGE_SIZE, SEEK_SET) != 0)
        {
          fprintf(stderr, "Error seeking in swap file\n");
          break;
        }

        //read the page from the swap file into physical memory
        if (fread(physical_memory[frame_number], sizeof(signed char), FRAME_SIZE, swap_file) == 0)
        {
          fprintf(stderr, "Error reading from swap file\n");
          break;
        }
      }
      //if there is a TLB miss, insert page/frame to the TLB
      insert_to_TLB(page_number, frame_number);
    }
    else
    {
      //increment TLB hits count
      TLB_hits++;
    }

    //compute physical address
    int physical_address = frame_number << 8;
    physical_address = physical_address | offset;

    //read value from physical memory (frame number and offset)
    read_value = physical_memory[frame_number][offset];

    printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, read_value);
  }
}

//FIFO algorithm
void fifo(char *address, FILE *swap_file, FILE *address_file)
{
  while (fgets(address, BUFFER_SIZE, address_file) != 0)
  {
    address_count++;

    logical_address = atoi(address);

    int page_number = logical_address >> PAGE_SHIFT;
    page_number = page_number & MASK;
    int offset = logical_address & MASK;

    int frame_number = check_TLB(page_number);

    if (frame_number == -1)
    {
      frame_number = check_page_table_fifo(page_number);

      if (frame_number == -1)
      {
        frame_number = frame_count++ % NUM_PAGES_MOD;
        insert_to_page_table_fifo(page_number, frame_number);

        faults_count++;

        if (fseek(swap_file, page_number * PAGE_SIZE, SEEK_SET) != 0)
        {
          fprintf(stderr, "Error seeking in swap file\n");
          break;
        }

        if (fread(physical_memory_mod[frame_number], sizeof(signed char), FRAME_SIZE, swap_file) == 0)
        {
          fprintf(stderr, "Error reading from swap file\n");
          break;
        }
      }
      insert_to_TLB(page_number, frame_number);
    }
    else
    {
      TLB_hits++;
    }

    int physical_address = frame_number << 8;
    physical_address = physical_address | offset;

    read_value = physical_memory_mod[frame_number][offset];

    printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, read_value);
  }
}

//LRU algorithm
void lru(char *address, FILE *swap_file, FILE *address_file)
{
  while (fgets(address, BUFFER_SIZE, address_file) != 0)
  {
    address_count++;

    logical_address = atoi(address);

    int page_number = logical_address >> PAGE_SHIFT;
    page_number = page_number & MASK;
    int offset = logical_address & MASK;

    int frame_number = check_TLB(page_number);

    if (frame_number == -1)
    {
      //check the page table for the page number
      int index = check_page_table_lru(page_number);

      //if it is not in the page table
      if (index == -1)
      {
        //create a frame number from the frame count and add it to the page table
        frame_number = insert_to_page_table_lru(page_number, frame_number);
        frame_count = (frame_count + 1) % NUM_FRAMES_MOD;

        faults_count++;

        if (fseek(swap_file, page_number * PAGE_SIZE, SEEK_SET) != 0)
        {
          fprintf(stderr, "Error seeking in swap file\n");
          break;
        }

        if (fread(physical_memory_mod[frame_number], sizeof(signed char), FRAME_SIZE, swap_file) == 0)
        {
          fprintf(stderr, "Error reading from swap file\n");
          break;
        }
      }
      else
      {
        //get frame number
        frame_number = page_table_mod[index].frame_number;
        //shift page table from index to the left
        shift_page_table_lru(index);
      }
      insert_to_TLB(page_number, frame_number);
    }
    else
    {
      TLB_hits++;
    }

    int physical_address = frame_number << 8;
    physical_address = physical_address | offset;

    read_value = physical_memory_mod[frame_number][offset];

    printf("Virtual address: %d Physical address: %d Value: %d\n", logical_address, physical_address, read_value);
  }
}