#/bin/bash
INPUT_FILE="../data/HS22.fasta"
INPUT_FORMAT="1"
APPEND_RC="0"
OUTPUT_FILE="${INPUT_FILE}.indexed"

./buildIndex ${INPUT_FILE} ${INPUT_FORMAT} ${APPEND_RC} ${OUTPUT_FILE}