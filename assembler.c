/*
    Name: Grant Reeder
    UTEID: ghr388
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255

static int PC, ORIG, SYMBOLS = 0;

const char* opcodes[] = {
    "add", "and", "halt", "jmp", "jsr", "jsrr", "ldb", "ldw", "lea", "not", "nop", "ret", "lshf", "rshfl", "rshfa", "rti", "stb", "stw", "trap", "xor", "brn", "brp", "brnp", "br", "brz", "brnz", "brzp", "brnzp"
};

const char* pseudoOps[] = {
    ".orig", ".fill", ".end"
};

enum {
   DONE, OK, EMPTY_LINE
};

typedef struct {
	int address;
	char label[MAX_LABEL_LEN + 1];
} TableEntry;
TableEntry symbolTable[MAX_SYMBOLS];

int readAndParse(FILE* pInFile, char* pLine, char **pLabel, char** pPseudoOp, char** pOpcode, char** pArg1, char** pArg2, char** pArg3, char** pArg4);
int toNum(char* pStr);
int labelToNum(char* label);
char* parseToHexString(char** lPseudoOp, char** lOpcode, char** lArg1, char** lArg2, char** lArg3, char** lArg4);
int regToHex(char* reg);
int isOpcode(char* lPtr);
int isPseudoOp(char* lPtr);
int isNum(char* str);

int main(int argc, char* argv[]) {
    if(argc != 3) {
        printf("Error: Missing arguments.\n");
        return 1;
    }

    FILE* inFile = fopen(argv[1], "r");
    FILE* outFile = fopen(argv[2], "w");

    if (!inFile) {
        printf("Error: Cannot open file %s\n", argv[1]);
        exit(4);
	}
    if (!outFile) {
        printf("Error: Cannot open file %s\n", argv[2]);
        exit(4);
    }

    char lLine[MAX_LINE_LENGTH + 1], *lLabel, *lPseudoOp, *lOpcode, *lArg1, *lArg2, *lArg3, *lArg4;
    int lRet;

    do {
        lRet = readAndParse(inFile, lLine, &lLabel, &lPseudoOp, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);

        if(lRet == OK) {
            if(strcmp(lPseudoOp, ".orig") == 0) {
                ORIG = PC = toNum(lArg1);
                continue;
            }

            if(strcmp(lLabel, "") != 0) {
                symbolTable[SYMBOLS].address = PC;
                strcpy(symbolTable[SYMBOLS].label, lLabel);
                SYMBOLS++;
            }

            PC++;
        }
    } while(lRet != DONE);

    rewind(inFile);
    PC = ORIG-1;
    
    do {
        lRet = readAndParse(inFile, lLine, &lLabel, &lPseudoOp, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);

        if(lRet == OK) {
            char* outLine = parseToHexString(&lPseudoOp, &lOpcode, &lArg1, &lArg2, &lArg3, &lArg4);
            fprintf(outFile, "%s\n", outLine);

            PC++;
        }
    } while(lRet != DONE);

    fclose(inFile);
    fclose(outFile);
}

int readAndParse(FILE* pInFile, char* pLine, char** pLabel, char** pPseudoOp, char** pOpcode, char** pArg1, char** pArg2, char** pArg3, char** pArg4) {
	char *lRet, *lPtr;
	int i;

	if(!fgets(pLine, MAX_LINE_LENGTH, pInFile))
		return(DONE);

	for(i = 0; i < strlen(pLine); i++)
		pLine[i] = tolower(pLine[i]);

	*pLabel = *pPseudoOp = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);
	lPtr = pLine;

	while(*lPtr != ';' && *lPtr != '\0' && *lPtr != '\n') 
	    lPtr++;
    *lPtr = '\0';

	if(!(lPtr = strtok(pLine, "\t\n ,")))
		return(EMPTY_LINE);

	if(!isOpcode(lPtr) && lPtr[0] != '.') {
		*pLabel = lPtr;
		if(!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
	}
    
    if(isPseudoOp(lPtr))
        *pPseudoOp = lPtr;
    else
        *pOpcode = lPtr;

    if(strcmp(lPtr, ".end") == 0)
        return(DONE);

	if(!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
    *pArg1 = lPtr;
    if(!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
	*pArg2 = lPtr;
	if(!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
	*pArg3 = lPtr;
	if(!(lPtr = strtok(NULL, "\t\n ,"))) return(OK);
	*pArg4 = lPtr;
	return(OK);
}

int toNum(char* pStr) {
    char* t_ptr;
    char* orig_pStr;
    int t_length,k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;
    if(*pStr == '#') { 
        pStr++;
        if(*pStr == '-') {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++) {
            if (!isdigit(*t_ptr)) {
                printf("Error: invalid decimal operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg)
            lNum = -lNum;
    
        return lNum;
    } else if(*pStr == 'x') {
        pStr++;
        if(*pStr == '-') {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for(k=0;k < t_length;k++) {
            if (!isxdigit(*t_ptr)) {
                printf("Error: invalid hex operand, %s\n",orig_pStr);
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16);
        lNum = (lNumLong > INT_MAX) ? INT_MAX : lNumLong;
        if(lNeg)
            lNum = -lNum;
        return lNum;
    } else {
        printf("Error: invalid operand, %s\n", orig_pStr);
        exit(4);
    }
}

int labelToNum(char* label) {
    for(int i = 0; i < SYMBOLS; i++) {
        if(strcmp(label, symbolTable[i].label) == 0)
            return symbolTable[i].address - (PC + 1);
    }
}

char* parseToHexString(char** pPseudoOp, char** pOpcode, char** pArg1, char** pArg2, char** pArg3, char** pArg4) {
    char* outLine;
    
    if(*pPseudoOp != NULL && (strcmp(*pPseudoOp, ".orig") == 0 || strcmp(*pPseudoOp, ".fill") == 0)) {
        sprintf(outLine, "0x%.4X", toNum(*pArg1) & 0xFFFF);
    } else if(strcmp(*pOpcode, "add") == 0 || strcmp(*pOpcode, "and") == 0) {
        int OP, DR, SR1, SR2, imm5;

        OP = strcmp(*pOpcode, "add") == 0 ? 0x1000 : 0x5000;
        DR = regToHex(*pArg1) << 9;
        SR1 = regToHex(*pArg2) << 6;

        if(isNum(*pArg3)) {
            imm5 = toNum(*pArg3) & 0x001F;
            sprintf(outLine, "0x%.4X", OP | DR | SR1 | 0x0020 | imm5);
        } else {
            SR2 = regToHex(*pArg3);
            sprintf(outLine, "0x%.4X", OP | DR | SR1 | SR2);
        }
    } else if(strncmp(*pOpcode, "br", 2) == 0) {
        int OP, n, z, p, PCoffset9;

        OP = strcmp(*pOpcode, "br") == 0 ? 0x0E00 : 0x0000;
        n = strchr(*pOpcode, 'n') != NULL ? 0x0800 : 0x0000;
        z = strchr(*pOpcode, 'z') != NULL ? 0x0400 : 0x0000;
        p = strchr(*pOpcode, 'p') != NULL ? 0x0200 : 0x0000;
        PCoffset9 = labelToNum(*pArg1) & 0x01FF;

        sprintf(outLine, "0x%.4X", OP | n | z | p | PCoffset9);
    } else if(strcmp(*pOpcode, "jmp") == 0) {
        int OP, BaseR;

        OP = 0xC000;
        BaseR = regToHex(*pArg1) << 6;

        sprintf(outLine, "0x%.4X", OP | BaseR);
    } else if(strcmp(*pOpcode, "jsr") == 0) {
        int OP, PCoffset11;

        OP = 0x4800;
        PCoffset11 = labelToNum(*pArg1) & 0x07FF;

        sprintf(outLine, "0x%.4X", OP | PCoffset11);
    } else if(strcmp(*pOpcode, "jsrr") == 0) {
        int OP, BaseR;

        OP = 0x4000;
        BaseR = regToHex(*pArg1) << 6;

        sprintf(outLine, "0x%.4X", OP | BaseR);
    } else if(strncmp(*pOpcode, "ld", 2) == 0) {
        int OP, DR, BaseR, offset6;

        OP = *(*pOpcode + 2) == 'b' ? 0x2000 : 0x6000;
        DR = regToHex(*pArg1) << 9;
        BaseR = regToHex(*pArg2) << 6;
        offset6 = toNum(*pArg3) & 0x003F;

        sprintf(outLine, "0x%.4X", OP | DR | BaseR | offset6);
    } else if(strcmp(*pOpcode, "lea") == 0) {
        int OP, DR, PCoffset9;

        OP = 0xE000;
        DR = regToHex(*pArg1) << 9;
        PCoffset9 = labelToNum(*pArg2) & 0x01FF;

        sprintf(outLine, "0x%.4X", OP | DR | PCoffset9);
    } else if(strcmp(*pOpcode, "not") == 0) {
        int OP, DR, SR;

        OP = 0x903F;
        DR = regToHex(*pArg1) << 9;
        SR = regToHex(*pArg2) << 6;

        sprintf(outLine, "0x%.4X", OP | DR | SR);
    } else if(strcmp(*pOpcode, "ret") == 0) {
        sprintf(outLine, "0x%.4X", 0xC1C0);
    } else if(strcmp(*pOpcode, "rti") == 0) {
        sprintf(outLine, "0x%.4X", 0x8000);
    } else if(strncmp(*pOpcode, "ls", 2) == 0 || strncmp(*pOpcode, "rs", 2) == 0) {
        int OP, DR, SR, amount4;

        OP = 0xD000;
        DR = regToHex(*pArg1) << 9;
        SR = regToHex(*pArg2) << 6;
        amount4 = toNum(*pArg3) & 0x000F;

        if(strcmp(*pOpcode, "rshfl") == 0)
            OP |= 0x0010;
        else if(strcmp(*pOpcode, "rshfa") == 0)
            OP |= 0x0030;

        sprintf(outLine, "0x%.4X", OP | DR | SR | amount4);
    } else if(strncmp(*pOpcode, "st", 2) == 0) {
        int OP, SR, BaseR, offset6;

        OP = strcmp(*pOpcode, "stb") == 0 ? 0x3000 : 0x7000;
        SR = regToHex(*pArg1) << 9;
        BaseR = regToHex(*pArg2) << 6;
        offset6 = toNum(*pArg3) & 0x003F;

        sprintf(outLine, "0x%.4X", OP | SR | BaseR | offset6);
    } else if(strcmp(*pOpcode, "trap") == 0) {
        int OP, trapvect8;

        OP = 0xF000;
        trapvect8 = toNum(*pArg1) & 0x00FF;

        sprintf(outLine, "0x%.4X", OP | trapvect8);
    } else if(strcmp(*pOpcode, "xor") == 0) {
        int OP, DR, SR1, SR2, imm5;

        OP = 0x9000;
        DR = regToHex(*pArg1) << 9;
        SR1 = regToHex(*pArg2) << 6;

        if(isNum(*pArg3)) {
            imm5 = toNum(*pArg3) & 0x001F;
            sprintf(outLine, "0x%.4X", OP | DR | SR1 | 0x0020 | imm5);
        } else {
            SR2 = regToHex(*pArg3);
            sprintf(outLine, "0x%.4X", OP | DR | SR1 | SR2);
        }
    } else if(strcmp(*pOpcode, "halt") == 0) {
        sprintf(outLine, "0x%.4X", 0xF025);
    } else if(strcmp(*pOpcode, "nop") == 0) {
        sprintf(outLine, "0x%.4X", 0x0000);
    }

    return outLine;
}

int regToHex(char* reg) {
    return reg[1] - '0';
}

int isOpcode(char* lPtr) {
    for(int i = 0; i < sizeof(opcodes) / sizeof(opcodes[0]); i++) {
        if(strcmp(lPtr, opcodes[i]) == 0) return 1;
    }
    return 0;
}

int isPseudoOp(char* lPtr) {
    for(int i = 0; i < sizeof(pseudoOps) / sizeof(pseudoOps[0]); i++) {
        if(strcmp(lPtr, pseudoOps[i]) == 0) return 1;
    }
    return 0;
}

int isNum(char* str) {
    return *str == 'x' || *str == '#';
}