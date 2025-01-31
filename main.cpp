#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)
class sArena {
	private:
		void* Memory;
		size_t FreePtr;
		size_t TotSize;
		size_t FreeSize;
		void memcpy(void* dest, void* src, size_t size){
			while(size--){
				((char*)dest)[size]=((char*)src)[size];
			}
		}		
	public:
		sArena(size_t hSize) {
			Memory=mmap(NULL, hSize,PROT_READ|PROT_WRITE,MAP_ANON|MAP_PRIVATE,-1,0);
			FreePtr=0;
			if (Memory==MAP_FAILED) handle_error("Heap creation failed");
			TotSize=hSize;
			FreeSize=hSize;
		}
		void* malloc(size_t size){
			if(size<=FreeSize){
				void* rPtr=(void*)((size_t)Memory+FreePtr);
				FreeSize-=size;
				FreePtr+=size;
				return rPtr;
			}
			handle_error("Out of memory");
		}
		void* realloc(void* ptr, size_t oldSize, size_t size){
			void* nptr=(*this).malloc(size);
			(*this).memcpy(nptr,ptr,oldSize);
			return nptr;
		}
		void free_heap(){
			munmap(Memory,TotSize);
		}
};

class sHeap {
	private:
		void* Memory;
		void* Cache;
		size_t CacheSize;
		size_t TotSize;
		size_t FreeSize;
		size_t Blockcount;
		struct Block {
			size_t data_size;
			Block* prev;
			Block* next;
			int is_free;
		};
		void initBlock(Block* block, size_t size){
			block->prev=NULL;
			block->next=NULL;
			block->data_size=size;
			block->is_free=1;
			Blockcount++;
			FreeSize-=sizeof(Block);
		}
		size_t try_merge_backwards(Block* block){
			return block->is_free&&block->prev!=NULL&&block->prev->is_free;
		}
		size_t try_merge_forwards(Block* block){
			return block->is_free&&block->next!=NULL&&block->next->is_free;
		}

		Block* mergeBlockForwards(Block* block){
			if(try_merge_forwards(block)){
				block->data_size+=block->next->data_size+sizeof(Block);
				FreeSize+=sizeof(Block);
				Blockcount--;
				block->next=block->next->next;
			}
			return block;
		}
		Block* mergeBlockBackwards(Block* block){
			if(try_merge_backwards(block)){
				block=block->prev;
				block->data_size+=block->next->data_size+sizeof(Block);
				FreeSize+=sizeof(Block);
				Blockcount--;
				block->next=block->next->next;
			}
			return block;
		}
		void divideBlock(Block* block, size_t size){
			Block* fBlock=(Block*)((size_t)block+sizeof(Block)+size);
			initBlock(fBlock, block->data_size-size-sizeof(Block));
			fBlock->prev=block;
			fBlock->next=block->next;
			block->data_size=size;
			if(fBlock->next!=NULL) fBlock->next->prev=fBlock;
			block->next=fBlock;
		}

		Block* mergeBlock(Block* block){
			if(try_merge_forwards(block)){
				block=mergeBlockForwards(block);
				block=mergeBlock(block);
			}
			else if(try_merge_backwards(block)){
				block=mergeBlockBackwards(block);
				block=mergeBlock(block);
			}
			return block;
		}
		Block* allocatePtr(size_t size, Block* block){
			if(block->is_free){
				if(block->data_size-size>sizeof(Block)+8){
					divideBlock(block, size);
					block->is_free=0;
					FreeSize-=block->data_size;
					return block;
				}
			}
			if(block->next!=NULL) return allocatePtr(size, block->next);
			return NULL;
		}
		Block* blockFromPointer(void* ptr){
			Block* block=(Block*)Memory;
			size_t tPtr=(size_t)ptr;
			while(block!=NULL){
				if(((size_t)block+sizeof(Block)+block->data_size)>tPtr&&(size_t)block<tPtr){return block;}
				block=block->next;
			}
			return NULL;
		}
		void memcpy(void* dest, void* src, size_t size){
			while(size--){
				((char*)dest)[size]=((char*)src)[size];
			}
		}		
	public:
		sHeap(size_t hSize, size_t cSize) {
			Memory=mmap(NULL, hSize,PROT_READ|PROT_WRITE,MAP_ANON|MAP_PRIVATE,-1,0);
			Cache=mmap(NULL, cSize, PROT_READ|PROT_WRITE,MAP_ANON|MAP_PRIVATE,-1,0);
			if (Memory==MAP_FAILED) handle_error("Heap creation failed");
			CacheSize=cSize;
			TotSize=hSize;
			FreeSize=hSize;
			Blockcount=0;
			initBlock((Block*)Memory,TotSize-sizeof(Block));
		}
		void* malloc(size_t size){return (void*)((size_t)allocatePtr(size, (Block*)Memory)+sizeof(Block));}
		void* realloc(void* ptr, size_t size){
			void* nptr=(*this).malloc(size);
			if(nptr==NULL){handle_error("Couldn't resize pointer ");}
			Block* cBlock=blockFromPointer(ptr);
			memcpy(nptr,ptr,cBlock->data_size);
			(*this).free(ptr);
			return nptr;
		}
		void free(void* ptr){
			Block* block=blockFromPointer(ptr);
			printf("%p:%p\n", block, ptr);
			if(block->is_free){handle_error("segfault");}
			FreeSize+=block->data_size;
			block->is_free=1;
			mergeBlock(block);
			printf("%lu\n", Blockcount);
		}
		void free_heap(){
			munmap(Memory,TotSize);
			munmap(Cache,CacheSize);
		}
};
int main(){
	sHeap Heap=sHeap(1024,128);
	int* a=(int*)Heap.malloc(sizeof(int));
	*a=1;
	printf("%d|%p\n",*a,a);
	int* b=(int*)Heap.malloc(sizeof(int));
	*b=1;
	printf("%d|%p\n",*b,b);
	b=(int*)Heap.realloc(b, 2*sizeof(int));
	printf("%d|%p\n",*b,b);

	Heap.free(b);
	Heap.free(a);
	Heap.free_heap();
	return 1;
}
