#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)

class sHeap {
	private:
		void* Memory;
		struct Heap {
			void* Memory;
			size_t Size; 
		};
		size_t Size;
		size_t BlocksizeMin;
		struct Block {
			size_t size;
			Block* prev;
			Block* next;
			int isFree;
		};
		struct Pointer {
			Block* block;
			size_t size;
		};

		Block* Blocks;
		Pointer* Pointers;
		void* bShift(size_t start){
			return ((void *)(start+sizeof(Block)));
		}
		void initBlock(Block* block, size_t size){
			block->prev=NULL;
			block->next=NULL;
			block->size=size;
			block->isFree=1;
		}
		void createPointer(){}
		

	public:
		sHeap(size_t hSize, size_t bSizeMin) {
			Memory=mmap(NULL, hSize,PROT_READ|PROT_WRITE,MAP_ANON|MAP_PRIVATE,-1,0);
			if (Memory==MAP_FAILED) handle_error("Heap creation failed");
			Size=hSize;
			BlocksizeMin=bSizeMin;
			initBlock(
		}
		
		
};
void* sAlloc(size_t size, size_t alignment){return NULL;}
void sFree(void* ptr){}
void* sResize(void* ptr, size_t size){return NULL;}
int main(){
	printf("aaa\n");
	return 1;
}
