/**
 * @author Fabio Cunial
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include "io.h"


char *DNA_ALPHABET = "acgt";
double DNA_ALPHABET_PROBABILITIES[4];
double LOG_DNA_ALPHABET_PROBABILITIES[4];


Concatenation loadFASTA(char *inputFilePath, uint8_t appendRC) {
	const char DO_NOT_PRINT = 'x';
	char c;
	uint8_t runOpen;
	int64_t i, j;
	uint64_t lineLength;  // Length of a line of the file
	uint64_t stringLength;  // Length of a FASTA sequence
	uint64_t bufferLength;
	uint64_t inputLength;  // Total length of the input, including non-DNA characters.
	uint64_t outputLength;  // Total length of the output, including non-DNA characters.
	uint64_t outputLengthDNA;  // Number of DNA characters in the output
	uint64_t outputLengthPrime, outputLengthDNAPrime;  // Temporary
	char *pointer, *buffer;
	FILE *file;
	Concatenation out;
	double tmpArray[4];
	
	file=fopen(inputFilePath,"r");
	if (file==NULL) {
		fprintf(stderr,"ERROR: cannot open input file \n");
		fclose(file);
		exit(EXIT_FAILURE);
	}
	
	// Loading the multi-FASTA input file
	for (i=0; i<4; i++) DNA_ALPHABET_PROBABILITIES[i]=0.0;
	buffer=(char *)malloc(BUFFER_CHUNK);
	bufferLength=BUFFER_CHUNK; 
	inputLength=0; outputLength=0; outputLengthDNA=0;
	c=fgetc(file);
	do {
		if (c!='>') {
			fprintf(stderr,"ERROR: input file not in FASTA format \n");
			fclose(file);
			exit(EXIT_FAILURE);
		}
		// Header
		c=fgetc(file);
		while (c!='\n' && c!=EOF) c=fgetc(file);
		if (c==EOF) {
			fprintf(stderr,"Omitting empty string \n");
			break;
		}
		// String
		stringLength=0; lineLength=0; runOpen=0;
		c=fgetc(file);
		while (c!=EOF && c!='>') {
			if (c=='\n') {
				c=fgetc(file);
				if (lineLength==0) fprintf(stderr,"Omitting empty line \n");
				lineLength=0;
				continue;
			}
			lineLength++; stringLength++; inputLength++; c=tolower(c);
			pointer=strchr(DNA_ALPHABET,c=='u'?'t':c);  // Handling RNA
			if (pointer==NULL) {
				if (!runOpen) {
					runOpen=1;
					c=CONCATENATION_SEPARATOR;
				}
				else c=DO_NOT_PRINT;
			}
			else {
				runOpen=0;
				outputLengthDNA++;
				DNA_ALPHABET_PROBABILITIES[pointer-DNA_ALPHABET]+=1.0;
			}
			if (c!=DO_NOT_PRINT) {
				if (outputLength==bufferLength) {
					bufferLength+=BUFFER_CHUNK;
					buffer=(char *)realloc(buffer,bufferLength*sizeof(char));
				}
				buffer[outputLength++]=c;
			}
			c=fgetc(file);
		}
		if (c!=EOF) {
			if (stringLength==0) fprintf(stderr,"Omitting empty string \n");
			else {
				if (outputLength==bufferLength) {
					bufferLength+=BUFFER_CHUNK;
					buffer=(char *)realloc(buffer,bufferLength*sizeof(char));
				}
				buffer[outputLength++]=CONCATENATION_SEPARATOR;
			}
		}
	} while (c!=EOF);
	fclose(file);
	
	// Appending reverse-complement, if needed.
	if (appendRC==1) {
		outputLengthPrime=(outputLength<<1)+1;
		outputLengthDNAPrime=outputLengthDNA<<1;
		if (bufferLength<outputLengthPrime) {
			bufferLength=outputLengthPrime;
			buffer=(char *)realloc(buffer,bufferLength*sizeof(char));
		}
		buffer[outputLength]=CONCATENATION_SEPARATOR;
		i=outputLength-1; j=outputLength+1;
		while (i>=0) {
			switch (buffer[i]) {
				case 'a': buffer[j]='t'; break;
				case 'c': buffer[j]='g'; break;
				case 'g': buffer[j]='c'; break;
				case 't': buffer[j]='a'; break;
				case 'u': buffer[j]='a'; break;  // Handling RNA
				case CONCATENATION_SEPARATOR: buffer[j]=CONCATENATION_SEPARATOR; break;
			}
			i--; j++;
		}
		for (i=0; i<4; i++) tmpArray[i]=DNA_ALPHABET_PROBABILITIES[3-i];
		for (i=0; i<4; i++) DNA_ALPHABET_PROBABILITIES[i]+=tmpArray[i];
	}
	else {
		outputLengthPrime=outputLength;
		outputLengthDNAPrime=outputLengthDNA;
	}
	
	// Computing probabilities
	for (i=0; i<4; i++) {
		DNA_ALPHABET_PROBABILITIES[i]/=outputLengthDNAPrime;
		LOG_DNA_ALPHABET_PROBABILITIES[i]=log(DNA_ALPHABET_PROBABILITIES[i]);
	}
	
	out.buffer=buffer;
	out.length=outputLengthPrime;
	out.inputLength=inputLength;
	out.hasRC=appendRC;
	return out;
}


Concatenation loadPlainText(char *inputFilePath, uint8_t appendRC) {
	const char DO_NOT_PRINT = 'x';
	char c;
	uint8_t runOpen;
	int64_t i, j;
	uint64_t bufferLength;
	uint64_t inputLength;  // Total length of the input, including non-DNA characters.
	uint64_t outputLength;  // Total length of the output, including non-DNA characters.
	uint64_t outputLengthDNA;  // Number of DNA characters in the output
	uint64_t outputLengthPrime, outputLengthDNAPrime;  // Temporary
	char *pointer, *buffer;
	FILE *file;
	Concatenation out;
	double tmpArray[4];
	
	file=fopen(inputFilePath,"r");
	if (file==NULL) {
		fprintf(stderr,"ERROR: cannot open input file \n");
		fclose(file);
		exit(EXIT_FAILURE);
	}
	
	// Loading the text file
	for (i=0; i<4; i++) DNA_ALPHABET_PROBABILITIES[i]=0.0;
	buffer=(char *)malloc(BUFFER_CHUNK);
	bufferLength=BUFFER_CHUNK;
	inputLength=0; outputLength=0; outputLengthDNA=0;
	c=fgetc(file); runOpen=0;
	while (c!=EOF) {	
		inputLength++; c=tolower(c);
		pointer=strchr(DNA_ALPHABET,c=='u'?'t':c);  // Handling RNA
		if (pointer==NULL) {
			if (!runOpen) {
				runOpen=1;
				c=CONCATENATION_SEPARATOR;
			}
			else c=DO_NOT_PRINT;
		}
		else {
			runOpen=0;
			outputLengthDNA++;
			DNA_ALPHABET_PROBABILITIES[pointer-DNA_ALPHABET]+=1.0;
		}
		if (c!=DO_NOT_PRINT) {
			if (outputLength==bufferLength) {
				bufferLength+=BUFFER_CHUNK;
				buffer=(char *)realloc(buffer,bufferLength*sizeof(char));
			}
			buffer[outputLength++]=c;
		}
		c=fgetc(file);
	}
	fclose(file);
	
	// Appending reverse-complement, if needed.
	if (appendRC==1) {
		outputLengthPrime=(outputLength<<1)+1;
		outputLengthDNAPrime=outputLengthDNA<<1;
		if (bufferLength<outputLengthPrime) {
			bufferLength=outputLengthPrime;
			buffer=(char *)realloc(buffer,bufferLength*sizeof(char));
		}
		buffer[outputLength]=CONCATENATION_SEPARATOR;
		i=outputLength-1; j=outputLength+1;
		while (i>=0) {
			switch (buffer[i]) {
				case 'a': buffer[j]='t'; break;
				case 'c': buffer[j]='g'; break;
				case 'g': buffer[j]='c'; break;
				case 't': buffer[j]='a'; break;
				case 'u': buffer[j]='a'; break;  // Handling RNA
				case CONCATENATION_SEPARATOR: buffer[j]=CONCATENATION_SEPARATOR; break;
			}
			i--; j++;
		}
		for (i=0; i<4; i++) tmpArray[i]=DNA_ALPHABET_PROBABILITIES[3-i];
		for (i=0; i<4; i++) DNA_ALPHABET_PROBABILITIES[i]+=tmpArray[i];
	}
	else {
		outputLengthPrime=outputLength;
		outputLengthDNAPrime=outputLengthDNA;
	}
	
	// Computing probabilities
	for (i=0; i<4; i++) {
		DNA_ALPHABET_PROBABILITIES[i]/=outputLengthDNAPrime;
		LOG_DNA_ALPHABET_PROBABILITIES[i]=log(DNA_ALPHABET_PROBABILITIES[i]);
	}
	
	out.buffer=buffer;
	out.length=outputLengthPrime;
	out.inputLength=inputLength;
	out.hasRC=appendRC;
	return out;
}


double getTime() {
	struct timeval ttime;
	gettimeofday(&ttime,0);
	return ttime.tv_sec+ttime.tv_usec*0.000001;
}