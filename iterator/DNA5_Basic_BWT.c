/**
 * @author Djamal Belazzougui, Fabio Cunial
 */
#include "DNA5_Basic_BWT.h"
#include "divsufsort64.h"


BwtIndex_t *newBwtIndex() {
	return (BwtIndex_t *)calloc(1,sizeof(BwtIndex_t));
}


void freeBwtIndex(BwtIndex_t *Basic_BWT) {
	if (Basic_BWT!=NULL) {
		if (Basic_BWT->indexedBWT!=NULL) {
			free_basic_DNA5_seq(Basic_BWT->indexedBWT);
			Basic_BWT->indexedBWT=NULL;
		}
		free(Basic_BWT);
	}
}


/**
 * Builds the BWT of T# from the suffix array of T built by divsufsort.
 * 
 * @param text the string T (without the final sharp), of length $length$;
 * @param Basic_BWT to set the position of the sharp;
 * @return a pointer to the BWT, or NULL if construction failed.
 */
static inline uint8_t *useDivsufsort(char *text, uint64_t length, BwtIndex_t *Basic_BWT) {
	uint32_t error;
	uint64_t i, textPosition;
	uint8_t *bwt = NULL;
	int64_t *suffixArray = NULL;
	
	suffixArray=(int64_t *)malloc(length*sizeof(int64_t));
	error=divsufsort64((uint8_t *)text,suffixArray,length);
	if (error) {
		free(suffixArray);
		return NULL;
	}
	bwt=(uint8_t *)malloc(length+1);
	bwt[0]=text[length-1];
	for (i=0; i<length; i++) {
		textPosition=suffixArray[i];
		if (textPosition==0) {
			Basic_BWT->sharpPosition=i+1;		
			bwt[i+1]=DNA_ALPHABET[0];
		}
		else bwt[i+1]=text[textPosition-1];
	}
	free(suffixArray);
	return bwt;
}


BwtIndex_t *buildBwtIndex(char *text, uint64_t length, uint32_t options) {
	uint8_t i;
	uint8_t *bwt;
	BwtIndex_t *bwtIndex = newBwtIndex();
	uint64_t tmpArray[4];
	
	bwt=useDivsufsort(text,length,bwtIndex);
	if (bwt==NULL) {
		freeBwtIndex(bwtIndex);
		return NULL;
	}
	
	// Indexing the BWT
	bwtIndex->indexedBWT=build_basic_DNA5_seq(bwt,length+1,&bwtIndex->size,tmpArray);
	if (bwtIndex->indexedBWT==NULL) {
		freeBwtIndex(bwtIndex);
		return NULL;
	}
	bwtIndex->cArray[0]=0;
	bwtIndex->cArray[1]=tmpArray[0]-1;  // Since # is replaced by an A in the BWT.
	for (i=2; i<=4; i++) bwtIndex->cArray[i]=bwtIndex->cArray[i-1]+tmpArray[i-1];
	bwtIndex->textLength=length;
	return bwtIndex;
}
