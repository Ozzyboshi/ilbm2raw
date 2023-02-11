
/*
Copyright (C) 2019-2023 Alessio Garzi <gun101@email.it>
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.
You should have received a copy of the GNU General Public
License along with this program; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

/*
The following code is based on a Magazine article from Sebastiano Vigna
You can read the whole article on Amiga Magazine number 11 (italian edition) in the "Transactor" section
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex.h>
#include <sys/stat.h>

typedef int8_t BYTE;
typedef uint8_t UBYTE;
typedef uint16_t UWORD;
typedef uint32_t ULONG;

typedef int16_t WORD;

#define MakeID(a, b, c, d) ((ULONG)(a) << 24L | (ULONG)(b) << 16L | (c) << 8 | (d))
#define FORM MakeID('F', 'O', 'R', 'M')
#define ID_ILBM MakeID('I', 'L', 'B', 'M')
#define ID_BMHD MakeID('B', 'M', 'H', 'D')
#define ID_CMAP MakeID('C', 'M', 'A', 'P')
#define ID_CAMG MakeID('C', 'A', 'M', 'G')
#define ID_BODY MakeID('B', 'O', 'D', 'Y')

typedef UBYTE Masking;
#define mskHasMask 1

typedef UBYTE Compression;
#define cmpByteRun1 1

typedef struct
{
	UWORD w, h;
	WORD x, y;
	UBYTE nPlanes;
	Masking masking;
	Compression compression;
	UBYTE pad1;
	UWORD transparentColor;
	UBYTE xAspect, yAspect;
	WORD pageWidth, pageHeight;
} BMapHeader;

UWORD swap_uint16(UWORD);
ULONG swap_uint32(ULONG);

BMapHeader BitMapHeader;
void printusage();
void printversion();
void print_bytes(void *ptr, int size);

#define UGetByte() (*source++)
#define UPutByte(c) (*dest++ = (c))

#define RowBytes(cols) ((((cols) + 15) / 16) * 2)

int unpackrow(UBYTE **pSource, UBYTE **pDest, UWORD srcBytes0, UWORD dstBytes0);
UBYTE *byterun1decompress(UBYTE *c, unsigned int, unsigned int *);
void convertToNonInterleaved(const char *, const char *);
void swapPaletteColors(const char *, BMapHeader, int *, int *, int);
void swapPaletteFile(const char *, int *, int *, int);
unsigned char checkSwap(unsigned char, int *, int *, int);
int ipow(int, int);
unsigned char getMask(int);

int VERBOSE = 0;
int ACE = 0;
int INTERLEAVED = 0;
int FORCE = 0;
int REMOVEBITPLANEFILES = 1;

int main(int argc, char **argv)
{
	ULONG t;
	long int h, l, ncol, i, x, y, n, f = 0, out, cont;
	char ColorMap[96], *p;
	unsigned long int ComAmg;
	UWORD yCont;
	UBYTE zCont;
	unsigned long int byteCounter;
	char cmd[1000];
	int opt;
	char *inputFile = NULL;
	const char *paletteFileName = NULL;
	const char *outputFileName = NULL;
	char aceFileHeader[] = "/tmp/ilbm2rawXXXXXX";
	regex_t regex;
	int reti;
	int swapPaletteCounter = 0;
	int swapPaletteX[100];
	int swapPaletteY[100];
	regmatch_t matches[3];
	struct stat statbuf;

	while ((opt = getopt(argc, argv, ":hvp:aiVs:fb")) != -1)
	{
		switch (opt)
		{
		case 'a':
			ACE = 1;
			break;
		case 'b':
			REMOVEBITPLANEFILES = 0;
			break;
		case 'f':
			FORCE = 1;
			break;
		case 'i':
			INTERLEAVED = 1;
			break;
		case 'h':
			printusage();
		case 'v':
			VERBOSE = 1;
			break;
		case 'p':
			paletteFileName = optarg;
			break;
		case 's':
			if (swapPaletteCounter < 100)
			{
				reti = regcomp(&regex, "^([0-9]+),([0-9]+)$", REG_EXTENDED);
				if (reti)
				{
					fprintf(stderr, "Could not compile regex\n");
					exit(1);
				}
				reti = regexec(&regex, optarg, 3, matches, 0);
				if (!reti)
				{
					memset(cmd, 0, sizeof(cmd));
					memcpy(cmd, optarg + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
					printf("Requested swap for color %s with ", cmd);
					swapPaletteX[swapPaletteCounter] = atoi(cmd);
					memset(cmd, 0, sizeof(cmd));
					memcpy(cmd, optarg + matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);
					printf("%s\n", cmd);
					swapPaletteY[swapPaletteCounter] = atoi(cmd);
					swapPaletteCounter++;
				}
				else if (reti == REG_NOMATCH)
				{
					puts("Wrong -s parameter, expecting format \"X,Y\" where X and Y are indexed palette color numbers");
					printusage();
				}
				else
				{
					fprintf(stderr, "Regex match failed\n");
					exit(1);
				}
				/* Free memory allocated to the pattern buffer by regcomp() */
				regfree(&regex);
			}
			break;
		case 'V':
			printversion();
		default:
			break;
		}
	}
	for (; optind < argc; optind++)
	{
		if (inputFile == NULL)
			inputFile = argv[optind];
		else if (outputFileName == NULL)
			outputFileName = argv[optind];
		else
			printusage();
	}
	if (VERBOSE)
		printf("Input file: %s\n", inputFile);
	if (VERBOSE && outputFileName)
		printf("Output file: %s\n", outputFileName);
	if (VERBOSE && paletteFileName)
		printf("Palette file: %s\n", paletteFileName);
	if (VERBOSE && ACE)
		printf("Ace mode enabled\n");
	if (VERBOSE && INTERLEAVED)
		printf("Interleaved mode enabled\n");

	if (inputFile == NULL || !strlen(inputFile))
		printusage();

	h = open(inputFile, O_RDONLY);
	if (h < 0)
	{
		perror("Iff file not found");
		exit(1);
	}

	(void)read(h, (void *)&t, 4);
	// if little endian
	t = swap_uint32(t);

	/* CHECK first 4 bytes equal "FORM" */

	if (t != FORM)
		exit(1);
	else if (VERBOSE)
		printf("Form tag found\n");

	/* Total length, not interesting */
	lseek(h, 4, SEEK_CUR);
	read(h, (void *)&t, 4);

	// if little endian
	t = swap_uint32(t);
	if (t != ID_ILBM)
		exit(1);
	else if (VERBOSE)
		printf("ILBM tag found\n");

	do
	{
		/* Read identificator and chunk length */
		if (!read(h, (void *)&t, 4))
			break;

		// if little endian
		t = swap_uint32(t);
		if (!read(h, (void *)&l, 4))
			break;

		// if little endian
		l = swap_uint32(l);

		// if (VERBOSE) print_bytes(&t,4);
		switch (t)
		{
		case ID_BMHD:
			if (VERBOSE)
				printf("Bitmap header found\n");
			read(h, (void *)&BitMapHeader, l);
			BitMapHeader.w = swap_uint16(BitMapHeader.w);
			BitMapHeader.h = swap_uint16(BitMapHeader.h);
			break;
		case ID_CMAP:
			if (VERBOSE)
				printf("Color map found\n");
			read(h, ColorMap, l);
			if (l % 2)
				lseek(h, 1, SEEK_CUR);
			ncol = l / 3;
			if (VERBOSE)
				printf("Number of colors : %ld\n", ncol);

			if (paletteFileName)
			{
				out = open(paletteFileName, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if (out < 0)
				{
					perror("Cant write palette output file");
					exit(1);
				}
				else if (VERBOSE)
					printf("palette file '%s' created\n", paletteFileName);
				if (ACE)
				{
					UBYTE tmp = (UBYTE)ncol;
					write(out, &tmp, 1);
				}
			}
			for (cont = 0; cont < l; cont += 3)
			{
				if (VERBOSE)
					printf("Color index %ld: %02x %02x %02x\n", cont / 3, (UBYTE)ColorMap[cont], (UBYTE)ColorMap[cont + 1], (UBYTE)ColorMap[cont + 2]);
				UBYTE color1 = (UBYTE)ColorMap[cont] & 0xF0;
				UBYTE color2 = (UBYTE)ColorMap[cont + 1] & 0xF0;
				UBYTE color3 = (UBYTE)ColorMap[cont + 2] & 0xF0;
				UBYTE firstByte = color1 >> 4;
				UBYTE secondByte = color2 | (color3 >> 4);
				UWORD finalValue = (UWORD)(firstByte << 8) | (UWORD)secondByte;
				UWORD finalValueSwapped = swap_uint16(finalValue);
				/*printf("Colore1: %02x\n",color1);
				printf("Colore2: %02x\n",color2);
				printf("Colore2: %02x\n",color3);
				printf("First byte %02x\n",firstByte);
				printf("Second byte %02x\n",secondByte);
				printf("final word %04x\n",finalValue);*/
				if (paletteFileName)
					write(out, &finalValueSwapped, 2);
				// if (VERBOSE) print_bytes((UBYTE*)&ColorMap[cont],1);
			}
			if (paletteFileName)
			{
				close(out);
				if (swapPaletteCounter)
				{
					swapPaletteFile(paletteFileName, swapPaletteX, swapPaletteY, swapPaletteCounter);
				}
			}
			break;
		case ID_CAMG:
			read(h, (void *)&ComAmg, 4);
			break;
		case ID_BODY:
			if (BitMapHeader.w/8*2*BitMapHeader.h*BitMapHeader.nPlanes == l && ((BitMapHeader.w%8) == 0))
			{
				BitMapHeader.w*=2;
				printf("Warning!!! Header double size of declared resolution, doubling width!!! New width is %dpx\n",BitMapHeader.w);
			}
			p = (void *)calloc(l, 1);
			read(h, p, l);
			if (VERBOSE)
				printf("Image size : %dX%d\n", BitMapHeader.w, BitMapHeader.h);
			if (VERBOSE)
				printf("Bitplanes : %d\n", BitMapHeader.nPlanes);

			// START OF cmpByteRun1 decompression
			if (BitMapHeader.compression == cmpByteRun1)
			{
				if (VERBOSE)
					printf("Bitmap compression is Byterun1\n");

				// Ace file header creation
				if (ACE)
				{
					int out2 = mkstemp(aceFileHeader);
					if (out < 0)
					{
						perror("Cant write ace output file");
						exit(1);
					}
					else if (VERBOSE)
						printf("Created ace header file %s\n", aceFileHeader);
					UWORD tmp = swap_uint16(BitMapHeader.w);
					write(out2, &tmp, 2); // Width in Pixel
					tmp = swap_uint16(BitMapHeader.h);
					write(out2, &tmp, 2);				   // Height in pixel
					write(out2, &BitMapHeader.nPlanes, 1); // Color depth
					UBYTE tmp2 = 0;
					write(out2, &tmp2, 1); // version
					tmp2 = (UBYTE)INTERLEAVED;
					write(out2, &tmp2, 1); // interleaved?
					close(out2);
				}

				// Decompress until we decoded BitMapHeader.h lines
				if (FORCE)
				{
					if (!stat(outputFileName,&statbuf) && unlink(outputFileName))
					{
						perror("Cant write output raw file");
						exit(1);
					}
				}
				if (VERBOSE) printf("Writing raw data to %s\n",outputFileName);
				out = open(outputFileName, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if (out < 0)
				{
					perror("Cant write output raw file");
					exit(1);
				}

				unsigned int bytesRead = 0;
				unsigned int bytesReadTotal = 0;
				UWORD planCounter = 0;
				for (planCounter = 0; planCounter < BitMapHeader.nPlanes; planCounter++)
				{
					UWORD yCounter = 0;
					for (yCounter = 0; yCounter < BitMapHeader.h; yCounter++)
					{
						UBYTE *decompressedData = byterun1decompress((UBYTE *)p + bytesReadTotal, RowBytes(BitMapHeader.w), &bytesRead);
						/*if (VERBOSE)
						{
							int i = 0;
							for (i = 0; i < 40; i++)
							{
								printf("%hhx ", decompressedData[i]);
							}
						}*/
						write(out, decompressedData, RowBytes(BitMapHeader.w));
						free(decompressedData);
						bytesReadTotal += bytesRead;
						/*printf("Bytes read %d\n",bytesRead);
						printf("Bytes read %d\n",bytesReadTotal);*/
						//						getchar();
					}
				}
				close(out);
				if (INTERLEAVED == 0)
				{
					convertToNonInterleaved(outputFileName, aceFileHeader);

					// Swap colors if requested from the user
					if (swapPaletteCounter)
					{
						swapPaletteColors(outputFileName, BitMapHeader, swapPaletteX, swapPaletteY, swapPaletteCounter);
					}
				}
			}
			// END OF cmpByteRun1 decompression

			// START OF case with no compression
			else if (!BitMapHeader.compression)
			{
				if (VERBOSE)
					printf("Bitmap not compressed, trying to parse as interleaved bitmap\n");

				// Ace file header creation
				if (ACE)
				{
					int out2 = mkstemp(aceFileHeader);
					if (out < 0)
					{
						perror("Cant write ace header output file");
						exit(1);
					}
					else if (VERBOSE)
						printf("Created ace header file %s\n", aceFileHeader);
					UWORD tmp = swap_uint16(BitMapHeader.w);
					write(out2, &tmp, 2); // Width in Pixel
					tmp = swap_uint16(BitMapHeader.h);
					write(out2, &tmp, 2);				   // Height in pixel
					write(out2, &BitMapHeader.nPlanes, 1); // Color depth
					UBYTE tmp2 = 0;
					write(out2, &tmp2, 1); // version
					tmp2 = (UBYTE)INTERLEAVED;
					write(out2, &tmp2, 1); // interleaved?
					close(out2);
				}

				if (VERBOSE)
					printf("Writing to %s\n", outputFileName);

				if (FORCE)
				{
					if (unlink(outputFileName))
					{
						perror("Cant write output raw file");
						exit(1);
					}
				}
				out = open(outputFileName, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if (out < 0)
				{
					perror("Cant write to output file");
					exit(1);
				}
				write(out, p, l);
				close(out);

				if (INTERLEAVED == 0)
				{
					for (byteCounter = 0, yCont = 0; yCont <= BitMapHeader.h; yCont++)
					{
						for (zCont = 0; zCont < BitMapHeader.nPlanes; zCont++)
						{
							snprintf(cmd, sizeof(cmd), "dd if=%s of=%s.%d bs=%d count=1 skip=%ld oflag=append conv=notrunc", outputFileName, outputFileName, zCont, BitMapHeader.w / 8, byteCounter);
							if (!VERBOSE)
								strcat(cmd, " 2>/dev/null");
							if (VERBOSE)
								printf("%s\n", cmd);
							system(cmd);
							byteCounter++;
						}
					}

					char cmd[1000];
					size_t len = 0;

					if (ACE && aceFileHeader)
						sprintf(cmd, "%s %s", "cat", aceFileHeader);
					else
						sprintf(cmd, "%s", "cat");
					len = strlen(cmd);
					for (zCont = 0; zCont < BitMapHeader.nPlanes; zCont++)
					{
						snprintf(cmd + len, sizeof(cmd) - len, " %s.%d", outputFileName, zCont);
						len = strlen(cmd);
					}
					snprintf(cmd + len, sizeof(cmd) - len, " > %s", outputFileName);
					system(cmd);
					if (VERBOSE)
						printf("Cmd: %s\n", cmd);

					for (zCont = 0; REMOVEBITPLANEFILES && zCont < BitMapHeader.nPlanes; zCont++)
					{
						snprintf(cmd, sizeof(cmd), "rm %s.%d", outputFileName, zCont);
						if (VERBOSE)
							printf("%s\n", cmd);
						system(cmd);
					}
					if (ACE && aceFileHeader)
					{
						snprintf(cmd, sizeof(cmd), "rm %s", aceFileHeader);
						system(cmd);
					}

					// Swap colors if requested from the user
					if (swapPaletteCounter)
					{
						swapPaletteColors(outputFileName, BitMapHeader, swapPaletteX, swapPaletteY, swapPaletteCounter);
					}
					// convertToNonInterleaved(outputFileName);
				}
			}
			else
			{
				printf("Bitmap compression is unknown (%d)\n", BitMapHeader.compression);
			}
			f = 1;
			break;
		default:
			lseek(h, l, SEEK_CUR);
			if (l % 2)
				lseek(h, 1, SEEK_CUR);
			break;
		}
	} while (!f);

	close(h);
	exit(0);
}

//! Byte swap unsigned short
UWORD swap_uint16(UWORD val)
{
	return (val << 8) | (val >> 8);
}

//! Byte swap short
int16_t swap_int16(int16_t val)
{
	return (val << 8) | ((val >> 8) & 0xFF);
}

//! Byte swap unsigned int
ULONG swap_uint32(ULONG val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}

//! Byte swap int
int32_t swap_int32(int32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | ((val >> 16) & 0xFFFF);
}

void print_bytes(void *ptr, int size)
{
	unsigned char *p = ptr;
	int i;
	for (i = 0; i < size; i++)
	{
		printf("%02hhX ", p[i]);
	}
	printf("\n");
}

UBYTE *byterun1decompress(UBYTE *data, unsigned int rowSize, unsigned int *bytesRead)
{
	UBYTE *decompressedData = NULL;
	UBYTE *dataPtr = data;
	BYTE curByte = 0;
	unsigned int colCounter = 0;
	const BYTE minus128 = -128;
	BYTE repeatCounter = 0;
	BYTE repeatLimit = 0;
	UBYTE writtenBytes = 0;

	decompressedData = malloc(rowSize);

	// while (colCounter<rowSize)
	// while (dataPtr<data+rowSize)
	while (writtenBytes < rowSize)
	{
		curByte = (BYTE)*dataPtr;
		// printf("leggo nuovo byte %d %hhx\n",curByte,(BYTE)curByte);

		// 0..127 copy the next n+1 bytes literally
		if (curByte >= 0)
		{
			// printf("Copia prossimi %hhu bytes\n",curByte+1);
			repeatLimit = curByte + 1;

			for (repeatCounter = 0; repeatCounter < repeatLimit; repeatCounter++)
			{
				dataPtr++;
				if (writtenBytes < rowSize)
					decompressedData[writtenBytes] = (BYTE)*dataPtr;
				else
					fprintf(stderr, "Warning buffer overflow\n");
				writtenBytes++;
				// printf("Copio %hhx - writtenbytes %d\n",(BYTE) *dataPtr,writtenBytes);
			}
			dataPtr++;
			// getchar();
		}
		// -128 NOP
		else if (curByte == minus128)
		{
			//printf("NOP found!\n");
			dataPtr++;
		}
		// -1..-127 => replicate next byte -n+1 times
		else
		{
			// printf("copia prossimo carattere %hhd\n",curByte);
			repeatLimit = curByte * -1 + 1;
			// printf("repeatlimit %d\n",repeatLimit);
			dataPtr++;

			for (repeatCounter = 0; repeatCounter < repeatLimit; repeatCounter++)
			{

				if (writtenBytes < rowSize)
					decompressedData[writtenBytes] = (BYTE)*dataPtr;
				else
					fprintf(stderr, "Warning buffer overflow\n");
				writtenBytes++;
				// printf("Copio %hhx writtenbytes %d\n",(BYTE) *dataPtr,writtenBytes);
			}
			dataPtr++;
			// getchar();
		}
	}
	*bytesRead = (unsigned int)(dataPtr - data);
	return decompressedData;
}

void convertToNonInterleaved(const char *fileName, const char *aceFileHeader)
{
	UWORD yCont;
	UBYTE zCont;
	unsigned long int byteCounter;
	char cmd[1000];
	size_t len = 0;
	struct stat statbuf;

	for (zCont = 0; FORCE && zCont < BitMapHeader.nPlanes; zCont++)
	{
		snprintf(cmd,sizeof(cmd),"%s.%d",fileName,zCont);
		printf("statto %s\n",cmd);
		if (!stat(cmd,&statbuf))
		{
			printf("unlink %s\n",cmd);
			if (unlink(cmd))
			{
				perror("Error unlinking");
				exit(0);
			}
		}
	}

	for (byteCounter = 0, yCont = 0; yCont < BitMapHeader.h; yCont++)
	{
		for (zCont = 0; zCont < BitMapHeader.nPlanes; zCont++)
		{
			snprintf(cmd, sizeof(cmd), "dd  if=%s of=%s.%d bs=%d count=1 skip=%ld oflag=append conv=notrunc", fileName, fileName, zCont, BitMapHeader.w / 8, byteCounter);
			if (!VERBOSE)
				strcat(cmd, " 2>/tmp/null");
			if (VERBOSE)
				printf("%s\n", cmd);
			if (WEXITSTATUS(system(cmd)))
			{
				printf("Error calculating bitplane\n");
				return;
			}
			byteCounter++;
		}
	}
	if (ACE && aceFileHeader)
		sprintf(cmd, "%s %s", "cat", aceFileHeader);
	else
		sprintf(cmd, "%s", "cat");
	len = strlen(cmd);
	for (zCont = 0; zCont < BitMapHeader.nPlanes; zCont++)
	{
		snprintf(cmd + len, sizeof(cmd) - len, " %s.%d", fileName, zCont);
		len = strlen(cmd);
	}
	snprintf(cmd + len, sizeof(cmd) - len, " > %s", fileName);
	system(cmd);
	if (VERBOSE)
		printf("Cmd: %s\n", cmd);

	for (zCont = 0; REMOVEBITPLANEFILES && zCont < BitMapHeader.nPlanes; zCont++)
	{
		snprintf(cmd, sizeof(cmd), "rm %s.%d", fileName, zCont);
		if (VERBOSE)
			printf("%s\n", cmd);
		system(cmd);
	}
	if (ACE && aceFileHeader)
	{
		snprintf(cmd, sizeof(cmd), "rm %s", aceFileHeader);
		system(cmd);
	}
}

/* This function reads a raw file, trys to find bitplane sections according to a bitmapheader data struct and
tries to swap colors according to swapPaletteX and swapPaletteY */
void swapPaletteColors(const char *outputFileName, BMapHeader BitMapHeader, int *swapPaletteX, int *swapPaletteY, int swapPaletteCounter)
{
	int i = 0;
	int contLine = 0;
	int contByte = 0;
	int contPixel = 0;
	int contBitplanes = 0;
	int contBitplanes2 = 0;
	unsigned char BYTE = 0;
	unsigned char NEWBYTE = 0;
	FILE *fd;
	unsigned char ptr;
	unsigned char mask;
	unsigned char newval;

	//printf("outputFileName:%s\n", outputFileName);
	fd = fopen(outputFileName, "rb+");
	if (!fd)
	{
		perror("Unable to open output file");
		exit(1);
	}

	int bytesPerLine = !(BitMapHeader.w % 8) ? BitMapHeader.w / 8 : 1 + BitMapHeader.w / 8;

	// For each line
	for (contLine = 0; contLine < BitMapHeader.h; contLine++)
	{
		// For each width byte
		for (contByte = 0; contByte < bytesPerLine; contByte++)
		{
			//printf("Processing byte %d\n", contByte);

			// For each pixel of the byte
			for (contPixel = 0; contPixel < 8; contPixel++)
			{
				//printf("Processing pixel %d\n", contPixel);

				BYTE = 0;
				// read data from bitplanes
				for (contBitplanes = 0; contBitplanes < BitMapHeader.nPlanes; contBitplanes++)
				{
					if (fseek(fd, (bytesPerLine * contLine) + contByte + (contBitplanes * BitMapHeader.w / 8 * BitMapHeader.h), SEEK_SET))
					{
						perror("Error seeking data\n");
						exit(1);
					}
					fread(&ptr, 1, 1, fd);
					//printf("vale %x\n", ptr);

					switch (contPixel)
					{
					case 0:
						mask = 0x80;
						break;
					case 1:
						mask = 0x40;
						break;
					case 2:
						mask = 0x20;
						break;
					case 3:
						mask = 0x10;
						break;
					case 4:
						mask = 0x08;
						break;
					case 5:
						mask = 0x04;
						break;
					case 6:
						mask = 0x02;
						break;
					case 7:
						mask = 0x01;
						break;
					}
					//printf("risultato %x\n", ptr & mask);
					if (ptr & mask)
					{
						BYTE += ipow(2, contBitplanes);
					}
				}
				//printf("BYte vale %d\n", BYTE);

				// check if BYE has to be swapped
				NEWBYTE = checkSwap(BYTE, swapPaletteX, swapPaletteY, swapPaletteCounter);
				if (NEWBYTE != BYTE)
				{
					//printf("Byte has to be swapped with %x\n", NEWBYTE);
					for (contBitplanes2 = 0; contBitplanes2 < BitMapHeader.nPlanes; contBitplanes2++)
					{
						if (fseek(fd, (bytesPerLine * contLine) + contByte + (contBitplanes2 * BitMapHeader.w / 8 * BitMapHeader.h), SEEK_SET))
						{
							perror("Error seeking data\n");
							exit(1);
						}
						fread(&ptr, 1, 1, fd);
						//printf("Replacing byte <<%x>> %x\n", ptr, getMask(contBitplanes2));

						if (NEWBYTE & getMask(contBitplanes2))
						{
							newval = ptr;
							newval |= 1 << 7 - contPixel;
							//printf("ciao 1 %x\n", newval);
						}
						else
						{
							newval = ptr;
							newval &= ~(1 << 7 - contPixel);
							//printf("ciao 0 %x\n", newval);
						}
						if (fseek(fd, (bytesPerLine * contLine) + contByte + (contBitplanes2 * BitMapHeader.w / 8 * BitMapHeader.h), SEEK_SET))
						{
							perror("Error seeking data\n");
							exit(1);
						}
						if (!fwrite(&newval, 1, 1, fd))
						{
							perror("Error writing new swapped color value");
						}
					}
				}
			}
		}
	}
	return;
}

unsigned char checkSwap(unsigned char BYTE, int *swapPaletteX, int *swapPaletteY, int swapPaletteCounter)
{
	int i = 0;
	for (i = 0; i < swapPaletteCounter; i++)
	{
		if ((int)BYTE == swapPaletteX[i])
			return swapPaletteY[i];
		if ((int)BYTE == swapPaletteY[i])
			return swapPaletteX[i];
	}
	return BYTE;
}

unsigned char getMask(int contPixel)
{
	unsigned char mask;
	switch (contPixel)
	{
	case 7:
		mask = 0x80;
		break;
	case 6:
		mask = 0x40;
		break;
	case 5:
		mask = 0x20;
		break;
	case 4:
		mask = 0x10;
		break;
	case 3:
		mask = 0x08;
		break;
	case 2:
		mask = 0x04;
		break;
	case 1:
		mask = 0x02;
		break;
	case 0:
		mask = 0x01;
		break;
	}
	return mask;
}

int ipow(int base, int exp)
{
	int result = 1;
	for (;;)
	{
		if (exp & 1)
			result *= base;
		exp >>= 1;
		if (!exp)
			break;
		base *= base;
	}

	return result;
}

void swapPaletteFile(const char *paletteFileName, int *swapPaletteX, int *swapPaletteY, int swapPaletteCounter)
{
	int i = 0;
	FILE *fd;
	unsigned char color1[2];
	unsigned char color2[2];
	unsigned char ptr[2];
	fd = fopen(paletteFileName, "rb+");
	if (!fd)
	{
		perror("Unable to open palette output file");
		exit(1);
	}
	for (i = 0; i < swapPaletteCounter; i++)
	{
		if (fseek(fd, swapPaletteX[i] * 2, SEEK_SET))
		{
			perror("Error seeking data\n");
			exit(1);
		}
		if (!fread(color1, 2, 1, fd))
		{
			fprintf(stderr, "Error getting color1 value\n");
			exit(1);
		}
		printf("color1 %x/%x\n", (unsigned int)color1[0], (unsigned int)color1[1]);

		if (fseek(fd, swapPaletteY[i] * 2, SEEK_SET))
		{
			perror("Error seeking data\n");
			exit(1);
		}
		if (!fread(color2, 2, 1, fd))
		{
			fprintf(stderr, "Error getting color2 value\n");
			exit(1);
		}
		printf("color2 %x/%x\n", color2[0], color2[1]);

		if (fseek(fd, swapPaletteX[i] * 2, SEEK_SET))
		{
			perror("Error seeking data\n");
			exit(1);
		}
		if (!fwrite(color2, 2, 1, fd))
		{
			perror("Error writing color 2 swap (are you giving me a wrong color index?)");
		}
		else
		{
			if (VERBOSE)
				printf("Value  %x/%x written at offset %d\n", color2[0], color2[1], swapPaletteX[i] * 2);
		}

		if (fseek(fd, swapPaletteY[i] * 2, SEEK_SET))
		{
			perror("Error seeking data\n");
			exit(1);
		}
		if (!fwrite(color1, 2, 1, fd))
		{
			perror("Error writing color 1 swap (are you giving me a wrong color index?)");
		}
		else
		{
			if (VERBOSE)
				printf("Value  %x/%x written at offset %d\n", color1[0], color1[1], swapPaletteY[i] * 2);
		}
	}
	fclose(fd);
	return;
}

void printusage()
{
	printf("Usage: %s inputIFFFile outputRAWFile [OPTIONS]\n", PACKAGE);
	printf("OPTIONS:\n");
	printf("	-a		   : Resulting file will be created according to ACE (Amiga C Engine) specifications (https://github.com/AmigaPorts/ACE)\n");
	printf("	-b         : Produce also bitplane files\n");
	printf("	-f 		   : Overwrite outputRAWFile if already created\n");
	printf("	-p outfile 	   : Write resulting palette into outfile\n");
	printf("	-i 		   : Output in interleaved mode (raw mode default)\n");
	printf("	-s X,Y		   : Swap image color and palette X with image color and palette of Y\n");
	printf("	-v		   : Be verbose\n");
	printf("	-V		   : Print version info\n");
	exit(1);
}
void printversion()
{
	printf("%s version '%s'\n", PACKAGE, VERSION);
	exit(0);
}
