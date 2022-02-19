/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Zhang Junyao",
    /* First member's full name */
    "19307130226",
    /* First member's email address */
    "19307130226@fudan.edu.cn",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define HSIZE 16
static void* heap_listp;
static void* heads[HSIZE];

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static int findhead(size_t size);
static void delete(void* bp);
static void insert(void* bp);
static void* place(void* bp,size_t size);
static int findhead(size_t size);
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    for (int i = 0; i < HSIZE; i++)
        heads[i] = NULL;
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void*)-1) return -1;
    PUT(heap_listp,0);
    PUT(heap_listp+(1*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(2*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(3*WSIZE),PACK(0,1));
    if(extend_heap(INITCHUNKSIZE)==NULL) return -1;
    return 0;
}
/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    if(size==0) return NULL;
    size=ALIGN(size+DSIZE);
    int head=findhead(size);
    void* t=NULL;
    for(;head<HSIZE;head++){
        t=heads[head];
        while((t!=NULL)&&size>GET_SIZE(HDRP(t)))
            t=SUCC(t);
        if(t!=NULL) break;
    }
    if(t!=NULL){
        t=place(t,size);
        return t;
    }
    if((t=extend_heap(MAX(size,CHUNKSIZE)))==NULL)
       return NULL;
    t=place(t,size);
    return t;
}
/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *bp, size_t size)
{
    void *new_bp=bp;
    int left;
    if (size == 0) return NULL;
    if (size <= DSIZE) size = 2 * DSIZE;
    else size = ALIGN(size + DSIZE);
    if ((left = GET_SIZE(HDRP(bp)) - size) == 0) return bp;
    else if ((left = GET_SIZE(HDRP(bp)) - size) == 0){
        bp=place(bp,size);
        return bp;
    }else if (!GET_ALLOC(HDRP(NEXT_BLKP(bp))) || !GET_SIZE(HDRP(NEXT_BLKP(bp)))){
        if ((left = GET_SIZE(HDRP(bp)) + GET_SIZE(HDRP(NEXT_BLKP(bp))) - size) < 0){
            if (extend_heap(MAX(-left, CHUNKSIZE)) == NULL)
                return NULL;
            left += MAX(-left, CHUNKSIZE);
        }
        delete(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size + left, 1));
        PUT(FTRP(bp), PACK(size + left, 1));
    }
    else{
        new_bp = mm_malloc(size);
        memcpy(new_bp, bp, GET_SIZE(HDRP(bp)));
        mm_free(bp);
    }
    return new_bp;
}

static void *extend_heap(size_t words)
{
    void *bp;
    words = ALIGN(words);
    if ((bp = mem_sbrk(words)) == (void *)-1) return NULL;
    PUT(HDRP(bp), PACK(words, 0));
    PUT(FTRP(bp), PACK(words, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    return coalesce(bp);
}

static void *coalesce(void *bp)
{
    size_t prev_alloc=GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size=GET_SIZE(HDRP(bp));
    if(prev_alloc&&next_alloc){
        insert(bp);
        return bp;
    }else if(!prev_alloc&&next_alloc){
        delete(PREV_BLKP(bp));
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp=PREV_BLKP(bp);
    }else if(prev_alloc&&!next_alloc){
        delete(NEXT_BLKP(bp));
        size+=GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }else{
        delete(PREV_BLKP(bp));
        delete(NEXT_BLKP(bp));
        size+=GET_SIZE(HDRP(PREV_BLKP(bp)))+GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp=PREV_BLKP(bp);
    }
    insert(bp);
    return bp;
}

static int findhead(size_t size){
     int i=0;
    for(;(i<HSIZE-1)&&(size>1);i++){
        size>>=1;
    }
    return i;
}

static void delete(void *bp)
{
    if(PRED(bp)==NULL){
       int head=findhead(GET_SIZE(HDRP(bp)));
       if(SUCC(bp)==NULL)
          heads[head]=NULL;
       else{
          PRED(SUCC(bp))= NULL;
          heads[head]=SUCC(bp);
       }
    }
    else if(SUCC(bp)==NULL)
            SUCC(PRED(bp))= NULL;
    else{
        SUCC(PRED(bp))=SUCC(bp);
        PRED(SUCC(bp))=PRED(bp);
    }
}

static void insert(void *bp)
{
    size_t size=GET_SIZE(HDRP(bp));
    int head = findhead(size);
    void *temp = NULL;
    void* t = heads[head];
    if(t==NULL){
        SUCC(bp)=NULL;
        PRED(bp)=NULL;
        heads[head] = bp;
        return;
    }
    if(size<=GET_SIZE(HDRP(t))){
        SUCC(bp)=t;
        PRED(t)=bp;
        PRED(bp)=NULL;
        heads[head] = bp;
        return;
    }
    while (t != NULL){
        if(GET_SIZE(HDRP(t))>=size) break;
        temp = t;
        t = SUCC(t);
    }
    if (t != NULL){
        SUCC(bp)=t;
        PRED(t)=bp;
        PRED(bp)=temp;
        SUCC(temp)=bp;  
    }else{
        SUCC(bp)=NULL;
        PRED(bp)=temp;
        SUCC(temp)=bp;
    }
}

static void *place(void *bp, size_t size)
{
    size_t total=GET_SIZE(HDRP(bp));
    size_t left=total-size;
    delete(bp);
    if(left<2*DSIZE){
        PUT(HDRP(bp),PACK(total,1));
        PUT(FTRP(bp),PACK(total,1));
        return bp;
    }
    if(size>=96){
        PUT(HDRP(bp),PACK(left,0));
        PUT(FTRP(bp),PACK(left,0));
        PUT(HDRP(NEXT_BLKP(bp)),PACK(size,1));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,1));
        insert(bp);
        return NEXT_BLKP(bp);
    }
    PUT(HDRP(bp),PACK(size,1));
    PUT(FTRP(bp),PACK(size,1));
    PUT(HDRP(NEXT_BLKP(bp)),PACK(left,0));
    PUT(FTRP(NEXT_BLKP(bp)),PACK(left,0));
    insert(NEXT_BLKP(bp));
    return bp;
}