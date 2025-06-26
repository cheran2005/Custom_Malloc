Custom Malloc / Free Implementation
===================================

This project is a custom memory allocator implemented in C, replicating core functionality of "malloc()" and "free()". It operates by managing heap memory manually using "sbrk()", and organizes allocations with a doubly linked list of memory blocks.

✅ Features Implemented
------------------------

📌 Basic Features (Must-Have)
- `my_malloc(size_t size)`  
  Custom implementation of `malloc()` that returns a pointer to a memory block of the requested size.
  
- `my_free(void *ptr)`  
  Frees a previously allocated block and marks it reusable.

- Linked List of Blocks  
  The heap is managed as a linked list of metadata headers representing each allocated or free block.

- Memory Acquisition via `sbrk()`  
  Memory is requested from the OS via the `sbrk()` system call for heap extension.

- Coalescing  
  Adjacent free blocks are merged into a single large block upon freeing to reduce fragmentation.

- Proper Alignment  
  All allocations are aligned to an 8-byte boundary using a macro:  
  `#define ALIGN(size) (((size) + 7) & ~7)`

- First-Fit Allocation Strategy  
  Searches the linked list for the first suitable free block large enough for the request.

- `my_calloc()` Equivalent  
  Allocates and zero-initializes memory using a wrapper that calls `my_malloc()` followed by `memset()`.

- `my_realloc()` Equivalent  
  Resizes an existing allocation, preserving content and reallocating if necessary.

- `my_malloc_stats()`  
  Prints memory usage statistics:
  - Total allocated and free memory
  - Number of blocks
  - Fragmentation ratio


🧪 Testing & Demonstration
--------------------------
- Test Application Includes:
  - Small (e.g. 16 bytes), medium (e.g. 256 bytes), and large (e.g. 2048+ bytes) allocations.
  - Demonstrates coalescing by freeing blocks and reusing space.
  - Basic performance measurement using timing functions (`clock()`).


🛠 How It Works
---------------
Each allocated block has a metadata header:

    typedef struct block_type {
        size_t size;
        unsigned int free;
        struct block_type *next;
        struct block_type *prev;
    } Block;

- The header is placed just before the user data.
- Block splitting occurs if the leftover space is enough for a new block + header.
- Coalescing merges adjacent free blocks to combat fragmentation.


🖥️ How to Compile and Run
--------------------------

    gcc -o my_malloc Main.c
    ./my_malloc

Ensure the file contains a variety of tests covering the allocator’s behavior.


📈 Future Enhancements (Not Implemented)
----------------------------------------
- `mmap()` for large allocations  
- Segregated free lists (slab-style)  
- Heap layout visualization using ASCII art


📄 License
----------
This project is for educational and learning purposes only.