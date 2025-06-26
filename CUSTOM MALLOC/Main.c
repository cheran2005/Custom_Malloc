/*
 * Main.c - Entire Program file
 * Author: Cheran Balakrishnan
 *
 * Description:
 * This program runs a custom malloc, free, calloc, and realloc. All memory allocated functions in C that are replicated. 
 *
 * Main functionalities:
 * - Custom implementation of `malloc()` that returns a pointer to a memory block of the requested size.
 * - Frees a previously allocated block and marks it reusable.
 * - The heap is managed as a linked list of metadata headers representing each allocated or free block.
 * - Memory is requested from the OS via the `sbrk()` system call for heap extension.
 * - Adjacent free blocks are merged into a single large block upon freeing to reduce fragmentation.
 * - All allocations are aligned to an 8-byte boundary.
 * - Searches the linked list for the first suitable free block large enough for the request.
 * - Custom implementation of 'calloc()' that allocates and zero-initalizes memory before returning of the requested size.
 * - Custom implementation of 'realloc()' that resizes an existing allocation block or reallocate the memory elsewhere.
 * - Prints memory usage statistics. 
 * 
 * Compile: gcc -o my_malloc Main.c
 * Run: ./my_malloc
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define ALIGNMENT 8//A constant used to ensure memory alignment of 8-byte

#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT -1))//round a size to the nearest multiple of 8 byte

typedef struct block_type{
    size_t size;//size of the block

    unsigned int free;//if the block is free or not

    struct block_type *next;//The next block when connecting in the linked list

    struct block_type *prev;//The previous block when connecting in the linked list so it can be a doubly linked list

}Block;

Block *head = NULL;//Setting the head of the doubly linked list to NULL

Block *last = NULL;//Setting the end of the doubly linked list to NULL



/**
 * my_malloc() - allocates and return a pointer to a memory block of requested size
 * 
 * size_t size: requested size in bytes from user
 * -----------------------------------------------------------------------------------  
 * 
 * Description: Custom implementation of malloc that allocates a block that is ensured to
 * have 8-byte alignment after aligning the requested size. It first searches for a suitable free block
 * using first-fit strategy. If none is found, then the function request more space directly from the OS to
 * expand the heap using sbrk(). It also manages metadata for managing a doubly linked list to track
 * all the allocated and free blocks. If any errors occur during this process, NULL is returned to the user.
 * 
 *           
 */
void *my_malloc(size_t size){

    size_t aligned_size = ALIGN(size);//align size to ensure it is rounded to nearest 8-byte for correct memory alignment


    
    Block *current = head;//Set current as the head block in the linked list


    //Continue looping through the linked list until the end of the list has been reached
    while (current != NULL){

        //check if the block is free to use
        if (current->free == 1 && current->size >= aligned_size){
            current->free = 0;

            //After checking if the block is free, check if the block can split with a new block being made from the extra space with atleast 8 bytes 
            if (current->size >= aligned_size + sizeof(Block) + ALIGNMENT){
                Block *new_block = (Block *)((char *)(current + 1) + aligned_size);//ensuring that the new_block takes up space in the heap that does not effect the current block
                
                new_block->size = current->size - aligned_size - sizeof(Block);
                new_block->prev = current;
                new_block ->next = current ->next;
                current->next = new_block;

                if (new_block->next != NULL){
                    new_block ->next->prev = new_block;
                }
                

            }
            //return the block with the correct size to the user
            return (void *)(current + 1);
        }

        //continue going through each block in the linked list
        current = current->next;

    }

    //after looping through the linked list, if none of the previously freed blocks has enough space to be reused, a new block will be created with new memory requested from the OS 
    //using sbrk() to add to the heap. This block is returned to the user and is set at the end of the linked list.
    
    void *mem_block = sbrk(aligned_size + sizeof(Block));
    if (mem_block == (void *)-1){
        perror("sbrk error");
        return NULL;
    }

    Block *allocated_block = (Block *)mem_block;

    allocated_block->free = 0;
    allocated_block->size = aligned_size;
    allocated_block->next = NULL;
    allocated_block->prev = NULL;

    //If the block created is the first block in the linked list
    if (head == NULL){
        head = allocated_block;
        last = allocated_block;
    }

    //Place the block at the end of the linked list if it is not the first block
    else{
        allocated_block ->prev = last;
        last->next = allocated_block;
        last = allocated_block;
    }
    
    //the + 1 ensures that the user only recieves space from the heap that is not apart of the meta data, since the + 1 skips past all the bytes that contain the meta data in the memory address.
    return ((void *)(allocated_block + 1));

}




/**
 * my_free() - free's a previously allocated block and marks it reusable
 * 
 * void *allocated_block: pointer to a previously allocated block
 * ------------------------------------------------------------------------------------  
 * 
 * Description: Custom implementation of free() that sets the pointer recieved to a resuable block by setting the meta data free variable to 1,
 * also checks adjecent nodes if they are also free to merge all free adjecent data blocks in the doubly linked list into one block to reduce 
 * fragmentation.
 * 
 *           
 */
void my_free(void *allocated_block){
    
    //check if the pointer recieved from the parameter is NULL
    if (allocated_block == NULL){
        fprintf(stderr,"invalid memory block\n");
        return;
    }

    //Changing the blocks meta data information, specifically the free variable to 1 since the block is being freed
    Block *free_block = (Block *)allocated_block - 1;
    free_block->free = 1;

    //Continue looping through any adjecent blocks that are also free to the given block and combine the size so all adjcent blocks can be treated as one large block.
    Block *current_fwd = free_block->next;
    while (current_fwd != NULL && current_fwd->free == 1){
        free_block->size += (sizeof(Block) + current_fwd->size);
        current_fwd  = current_fwd ->next;
    }

    free_block->next = current_fwd;

    if (current_fwd != NULL){
        current_fwd ->prev = free_block;
    }


    //continue looping through blocks adjacent to the given block in the left direction or previous direction, for each adjacent block, update the size to include all free adjacent blocks size in the right direction of the block.
    //Continue doing this until there is no more free blocks, combining adjacent blocks into one large block for more reusability.
    Block *current_bck = free_block->prev;
    while (current_bck != NULL && current_bck->free == 1){

        current_bck->size += sizeof(Block)+free_block->size;
        current_bck->next = free_block->next;
        
        if (free_block->next != NULL){
            free_block->next->prev = current_bck;
        }

        free_block = current_bck;
        current_bck = current_bck->prev;

    }

    return;


}



/**
 * my_calloc() - dynamically allocates a block of memory for an array that has value elements, each with a size amount of bytes, all bytes being initalized to 0.
 * 
 * size_t value: number of elements in the array
 * 
 * size_t size: size of each element in the array
 * ------------------------------------------------------------------------------------  
 * 
 * Description: Custom implementation of calloc that uses my_malloc() to dynamically allocate a block of memory in a pointer requested from the user.
 * Then set each byte in the pointer to 0 so the entire array is initalized to 0. Then the initalized pointer is returned to the user. If any errors occur during this process, 
 * NULL is returned to the user.
 * 
 *           
 */

void *my_calloc(size_t value,size_t size){ 

    //Check for edge cases
    if (size == 0 || value == 0){
        return NULL;
    }

    //check for integer overflow when calculating the size
    size_t Total_size = size * value;
    if (Total_size/value != size ){
        return NULL;
    }

    //use my_malloc() to allocate a block of memory to use
    void *new_pointer = my_malloc(Total_size);

    //check for any my_malloc() error
    if (new_pointer == NULL){
        fprintf(stderr,"my_calloc failed\n");\
        return NULL;
    }

    //dereference the new_pointer to a char type so each byte from the memory address can be initialized to 0
    char *ptr = (char *)new_pointer;

    for (size_t i = 0; i <Total_size; i++){

        ptr[i] = 0;

    }

    //return the initialized pointer to the user
    return new_pointer;

}



/**
 * my_realloc() - dynamically resizes a previously allocated block of memory.
 * 
 * void *ptr: previously allocated block of memory
 * 
 * size_t size: new size value to resize void *ptr
 * ------------------------------------------------------------------------------------  
 * 
 * Description: Custom implementation of realloc that takes in a previously allocated block of memory. It first checks if the change in size is to shrink the block,
 * then the block of memory meta data 'size' is changed to the new size value. If the size value is larger than the meta data 'size' value then the function
 * uses my_malloc() to create a new block of memory of the requested size change and then copy's *ptr memory into the block using memcpy(). After copying the *ptr block
 * is then freed and the new larger block of memory with the copyed values of *ptr is returned to the user. If any errors occur during this process, NULL is returned to the user.
 * 
 *           
 */
void *my_realloc(void *ptr,size_t size){


    size_t aligned_size = ALIGN(size);//align size to ensure it is rounded to nearest 8-byte for correct memory alignment
    if (ptr == NULL){
        return my_malloc(size);
    }

    //if the user wants the block to have a size of 0 then the block is freed
    if (size == 0){
        my_free(ptr);
        return NULL;
    }

    //Dereference the block and subtract the ptr by 1 to access the meta data in the address
    Block *current = (Block *)ptr -1;

    //first check if the change in size is less than the original size of the current block, if it is less then only change the meta data information on the size of the block
    if (current->size >= aligned_size){

        //After changing the orginal size, check if the left over remaining memory is enough to create a block of atleast 8 bytes
        if (current->size >= aligned_size + sizeof(Block) + ALIGNMENT){

            //Creating a block with the unused memory and ensure it is adjacent to the current block
            Block *new_block = (Block *)((char *)(current + 1) + aligned_size);
            new_block->free = 1;
            new_block->prev = current;
            new_block->next = current->next;
            
            new_block ->size = current->size - aligned_size - sizeof(Block);
            if (current->next != NULL){
                current->next->prev = new_block;
            }
            
            current->next = new_block;
        }

        //setting the size to the new size
        current->size = aligned_size;

        return ptr;

    }

    //If the size is larger than the current size, then a new block is created with the my_malloc function
    else if(current->size < aligned_size ){
        void *new_ptr = my_malloc(aligned_size);

        //check for any my_malloc() errors
        if (new_ptr == NULL){
            fprintf(stderr,"realoc error\n");
            return NULL;
        }

        //copy memory of the old block to the new block
        memcpy(new_ptr, ptr,current->size);

        //after copying, free the old block of memory
        my_free(ptr);

        return new_ptr;

    }

    //If the memory address is invalid
    else{
        fprintf(stderr,"invalid ptr\n");
        return NULL;
    }
    
}


/**
 * my_malloc_stats() - displays stats on all the dynamically allocated memory occurring in the program
 * 
 * ------------------------------------------------------------------------------------  
 * 
 * Description: displays in the terminal the total amount of memory blocks, free memory blocks, used memory blocks, amount of bytes used, and amount of bytes freed.
 * In addition it also outputs the fragmentation occuring in the memory which is the percentage of free bytes from the total bytes dynamically allocated.
 * 
 *           
 */
void my_malloc_stats(){

    //Information on the current heap space
    size_t total_blocks = 0;

    size_t free_blocks = 0;

    size_t used_blocks = 0;

    size_t used_bytes = 0;

    size_t free_bytes = 0;

    Block *current = head;

    //loop through the linked list to update the variables recording the heap space information
    while (current != NULL){

        total_blocks++;

        if (current->free == 0){
            used_blocks++;
            used_bytes = current->size;
        }

        else if (current->free == 1){
            free_blocks++;
            free_bytes = current->size;
        }

        current = current->next;

    }

    //output information to the terminal
    printf("\n============Malloc Stats==============\n");
    printf("Total Blocks:               %zu\n", total_blocks);
    printf("Used Blocks:                %zu\n", used_blocks);
    printf("Free Blocks:                %zu\n", free_blocks);
    printf("Used Memory (B):            %zu\n", used_bytes);
    printf("Free Memory (B):            %zu\n", free_bytes);
    printf("Total Memory (B):           %zu\n", used_bytes + free_bytes);

    //checking if any memory is allocated before doing fragmentation calculation
    if ((used_bytes + free_bytes)>0){

        //calculate the fragmentation in the heap and output to the terminal
        float fragmentation = 100.0f *((float)(free_bytes)/(float)(free_bytes + used_bytes));
        printf("fragmentation               %.2f%%\n", fragmentation);

    }

    //If no memory is allocated
    else{
        
        printf("fragmentation           N/A\n");
    }

    printf("=====================================\n\n");

    return;
}



/**
 * main() - Main program to demonstrate the custom implementation of different memory allocation functions in C
 * ---------------------------------------------------
 * A demonstration of the following functions:
 * 
 * - my_malloc()
 * - my_calloc()
 * - my_realloc()
 * - my_free()  
 * - my_malloc_stats()    
 * 
 */
int main(){

    printf("----- Custom malloc demo -----\n");

    // 1. Allocate 32 bytes
    void *ptr1 = my_malloc(32);
    if (ptr1 != NULL) {
        strcpy(ptr1, "Hello, custom malloc!");
        printf("ptr1: %s\n", (char *)ptr1);
    }

    my_malloc_stats();

    // 2. Allocate and zero 4 integers (calloc)
    int *arr = (int *)my_calloc(4, sizeof(int));
    if (arr != NULL) {
        printf("arr: ");
        for (int i = 0; i < 4; i++) {
            printf("%d ", arr[i]); 
        }
        printf("\n");
    }
    my_malloc_stats();

    // 3. Reallocate ptr1 to a larger size
    ptr1 = my_realloc(ptr1, 64);
    if (ptr1 != NULL) {
        strcat(ptr1, " Extended!");
        printf("ptr1 after realloc: %s\n", (char *)ptr1);
    }
    my_malloc_stats();

    // 4. Free both
    my_free(ptr1);
    my_free(arr);
    my_malloc_stats();

    return 0;
}