/************************************************************************
COPYRIGHT (C) STMicroelectronics 2005

Source file name : allocator_simple.h
Author :           Nick

Implementation of the pure virtual class used to control the allocation
of portions of a buffer.

Note this is the simplest of implementations, it is definitely not the best
Note Allocations are returned as offsets from the base of the buffer 
     (IE NULL == the base), this is because the allocator is used to allocate 
     blocks from buffers that will have multiple address mappings (physical cached etc)
Note this implementation will not return a block of memory that crosses a 64Mb 
     physical address boundary


Date        Modification                                    Name
----        ------------                                    --------
29-Jun-05   Created                                         Nick
25-Feb-08   Added code to avoid 64Mb address boundaries     Nick

************************************************************************/

#ifndef H_ALLOCATOR_SIMPLE
#define H_ALLOCATOR_SIMPLE

//

#include "osinline.h"
#include "allocator.h"

//

//#define ALLOCATOR_SIMPLE_MAX_BLOCKS	32
#define ALLOCATOR_SIMPLE_MAX_BLOCKS	256

typedef struct AllocatorSimpleBlock_s
{
    bool		 InUse;
    unsigned int	 Size;
    unsigned char	*Base;
} AllocatorSimpleBlock_t;

//

class AllocatorSimple_c : public Allocator_c
{
private:

    bool			 BeingDeleted;
    bool			 InAllocate;

    unsigned int		 BufferSize;
    unsigned int 		 SegmentSize;
    unsigned char		*PhysicalAddress;
    unsigned int		 HighestUsedBlockIndex;
    AllocatorSimpleBlock_t	 Blocks[ALLOCATOR_SIMPLE_MAX_BLOCKS];

    OS_Mutex_t			 Lock;
    OS_Event_t			 EntryFreed;

public:

    AllocatorSimple_c( 		unsigned int	 BufferSize,
				unsigned int	 SegmentSize,
				unsigned char	*PhysicalAddress );
    ~AllocatorSimple_c( 	void );

    AllocatorStatus_t Allocate( unsigned int	  Size,
				unsigned char	**Block,
				bool		  NonBlocking	= false );

    AllocatorStatus_t Free( 	void );

    AllocatorStatus_t Free( 	unsigned int	  Size,
				unsigned char	 *Block );

    AllocatorStatus_t LargestFreeBlock( 	
				unsigned int	 *Size );
};
#endif

