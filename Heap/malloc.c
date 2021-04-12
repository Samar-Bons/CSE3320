#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allcated memory   */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */
struct _block *last_allocation = NULL; /* Most recently allocated block for Next-Fit algorithm */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

#if defined BEST && BEST == 0
  
   /* Best fit */
   struct _block *best = NULL;
   size_t sizeDiff = INT_MAX;
   while(curr != NULL)
   {
      *last = curr;
      if(curr->free && (curr->size - size) < sizeDiff)
      {
         best = curr;
         sizeDiff = curr->size - size;
      }
      curr = curr->next;

   }
   if(best != NULL)
   {
      curr = best;
   }

#endif

#if defined WORST && WORST == 0
   
   /* Worst-Fit Algorithm */
   struct _block *worstBlock = NULL;
   size_t largestSize = 0;

   while(curr != NULL)
   {
      *last = curr;
      if(curr && (curr->size - size) > largestSize)
      {
         worstBlock = curr;
         largestSize = curr->size - size;
      }
      curr = curr->next;
   }
   if(worstBlock != NULL)
   {
      curr = worstBlock;
   }

#endif

#if defined NEXT && NEXT == 0
   
   while(last_allocation && !(last_allocation->free && last_allocation->size >= size))
   {
      *last = last_allocation;
      last_allocation = last_allocation->next;

   }
   if( !last_allocation) last_allocation = curr;

   while(last_allocation && !(last_allocation->free && last_allocation->size >= size))
   {
      *last = last_allocation;
      last_allocation = last_allocation->next;

   }

   curr = last_allocation;

   

#endif

   return curr;
}

/* 
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to prev _block */
   if (last) 
   {
      last->next = curr;
      num_blocks++;

   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   //num_blocks++;
   num_grows++;
   //max_heap+=size;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   /* TODO: Split free _block if possible */ 
   if(next != NULL)
   {
      num_reuses++;
   }

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
      max_heap+=size;
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }

   num_mallocs++;
   
   /* Mark _block as in use */
   next->free = false;

   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}

void *calloc(size_t nmemb, size_t size)
{
   if(nmemb == 0 || size == 0)
   {
      return NULL;
   }

   void *ptr = malloc(nmemb * size);

   if(ptr == NULL) return NULL;

   memset(ptr,0,nmemb);
   return ptr;
}

void *realloc(void *ptr, size_t size)
{
      if (size == 0)
      {
         free(ptr);
         return NULL;
      }
      if( ptr == NULL)
      {
         return malloc(size);
      }

      void *temp = malloc(size);
      memcpy(temp,ptr,size);
      return temp;

}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   num_frees++;


   /* TODO: Coalesce free _blocks if needed */
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
