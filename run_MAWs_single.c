#include <stdio.h>
#include <stdlib.h>
#include "./malloc_count/malloc_count.h"  // For measuring memory usage
#include "./io/io.h"
#include "./iterator/DNA5_Basic_BWT.h"
#include "./callbacks/MAWs_single.h"


/** 
 * 1: input file path;
 * 2: append reverse-complement (1/0);
 * 3: minimum MAW length;
 * 4: write MAWs to a file (1/0);
 * 5: assigns a score to each MAW (1/0); used only if MAWs are written to a file;
 * 6: output file path (read only if the previous argument is 1).
 */
int main(int argc, char **argv) {
	char *INPUT_FILE_PATH = argv[1];
	const unsigned char APPEND_RC = atoi(argv[2]);
	const unsigned int MIN_MAW_LENGTH = atoi(argv[3]);
	const unsigned char WRITE_MAWS = atoi(argv[4]);
	const unsigned char COMPUTE_SCORE = atoi(argv[5]);
	char *OUTPUT_FILE_PATH = NULL;
	if (WRITE_MAWS==1) OUTPUT_FILE_PATH=argv[6];
	double t, tPrime, loadingTime, indexingTime, processingTime;
	unsigned int nMAWs;
	Concatenation sequence;
	Basic_BWT_t *bbwt;
	FILE *file;

	t=getTime();
	sequence=loadFASTA(INPUT_FILE_PATH,APPEND_RC);
	tPrime=getTime();
	loadingTime=tPrime-t;
	
	t=tPrime;
	bbwt=Build_BWT_index_from_text(sequence.buffer,sequence.length,Basic_bwt_free_text);
	indexingTime=getTime()-t;
	
	// Erasing output file
	if (WRITE_MAWS==1) {
		file=fopen(OUTPUT_FILE_PATH,"w");
		fclose(file);
	}
	
	t=getTime();
	nMAWs=find_MAWs_single(bbwt,sequence.length,MIN_MAW_LENGTH,WRITE_MAWS,COMPUTE_SCORE,OUTPUT_FILE_PATH);
	processingTime=getTime()-t;
	free_Basic_BWT(bbwt);
	
	if (WRITE_MAWS==1) fclose(file);
	
	printf( "%ld,%ld,%d,%d,%f,%f,%f,%ld,%d \n", 
	        sequence.inputLength,
	        sequence.length,
			sequence.hasRC,
			MIN_MAW_LENGTH,
			loadingTime,
			indexingTime,
			processingTime,
			malloc_count_peak(),
			nMAWs
	      );
	return 0;
}