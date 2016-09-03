#include "MD5.h"

int const DATA_GROUP_SIZE = 64;

unsigned int const S[] = {
	7, 12, 17, 22,
	5, 9, 14, 20,
	4, 11, 16, 23,
	6, 10, 15, 21
};

unsigned int const T[] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

unsigned int shift(unsigned int x, unsigned int n) {
	return ((x << n) | (x >> (32 - n)));
}

unsigned int F(unsigned int X, unsigned int Y, unsigned int Z) {
	return (X & Y) | ((~X) & Z);
}

unsigned int G(unsigned int X, unsigned int Y, unsigned int Z) {
	return (X & Z) | (Y & (~Z));
}

unsigned int H(unsigned int X, unsigned int Y, unsigned int Z) {
	return X ^ Y ^ Z;
}

unsigned int I(unsigned int X, unsigned int Y, unsigned int Z) {
	return Y ^ (X | (~Z));
}

void dataOperation(unsigned int *a, unsigned int *b, unsigned int *c, unsigned int *d, unsigned int const *m) {
	unsigned int i;
	unsigned int f, g, h;
	unsigned int aTemp = *a;
	unsigned int bTemp = *b;
	unsigned int cTemp = *c;
	unsigned int dTemp = *d;

	for (i = 0; i < 64; i++) {
		if (i < 16) {
			f = F(*b, *c, *d);
			g = i;
			h = i % 4;
		} else if (i < 32) {
			f = G(*b, *c, *d);
			g = (5 * i + 1) % 16;
			h = i % 4 + 4;
		} else if (i < 48) {
			f = H(*b, *c, *d);
			g = (3 * i + 5) % 16;
			h = i % 4 + 8;
		} else {
			f = I(*b, *c, *d);
			g = (7 * i) % 16;
			h = i % 4 + 12;
		}
		unsigned int tmp = *d;
		*d = *c;
		*c = *b;
		*b = *b + shift((*a + f + m[g] + T[i]), S[h]);
		*a = tmp;
	}

	*a = *a + aTemp;
	*b = *b + bTemp;
	*c = *c + cTemp;
	*d = *d + dTemp;
}

int IsLittleEndian() {
	unsigned int i = 0x80000000;
	return (*(unsigned char*)(&i) == 0x00);
}

size_t FillBuf(char const **out, char const *in, size_t inLen) {
	int i;
	int j;

	size_t bitLen = inLen * 8;
	size_t fillLen = 64 - (inLen - 56) % 64;
	size_t filledLen = inLen + fillLen + 8;

	char *outBuf = (char*)calloc(filledLen, sizeof(char));
	memcpy(outBuf, in, inLen);

	outBuf[inLen] = 0x80;

	j = sizeof(inLen);
	if (IsLittleEndian()) {
		for (i = 0; i < j; i++) {
			memcpy(outBuf + filledLen - 8 + i, (char*)(&bitLen) + i, 1);
		}
	} else {
		for (i = 0; i < j; i++) {
			memcpy(outBuf + filledLen - 8 + i, (char*)(&bitLen) + j - i - 1, 1);
		}
	}

	*out = outBuf;
	return filledLen;
}

void SetABCD(unsigned int *A, unsigned int *B, unsigned int *C, unsigned int *D) {
	unsigned char data[] = {
		0x01, 0x23, 0x45, 0x67,
		0x89, 0xAB, 0xCD, 0xEF,
		0xFE, 0xDC, 0xBA, 0x98,
		0x76, 0x54, 0x32, 0x10
	};

	memcpy(A, data, 4);
	memcpy(B, data + 4, 4);
	memcpy(C, data + 8, 4);
	memcpy(D, data + 12, 4);
}

void SetM(unsigned char *out, unsigned char const *in) {
	int i, j;

	if (IsLittleEndian()) {
		memcpy(out, in, DATA_GROUP_SIZE);
	} else {
		for (i = 0; i < DATA_GROUP_SIZE / 4; i++) {
			for (j = 0; j < 4; j++) {
				memcpy(out + i * 4 + j, in + i * 4 + 3 - j, 4);
			}
		}
	}
}

void GetMD5(char *out, char const *in, size_t inLen) {
	unsigned int i;
	size_t filledLen;
	char *filledData;
	unsigned int A, B, C, D;
	unsigned int *M = (unsigned int*)malloc(DATA_GROUP_SIZE);

	filledLen = FillBuf(&filledData, in, inLen);

	SetABCD(&A, &B, &C, &D);

	for (i = 0; i < filledLen / DATA_GROUP_SIZE; i++) {
		SetM(M, filledData + i * 64);

		dataOperation(&A, &B, &C, &D, M);
	}

	printf("%0X%0X%0X%0X", A, B, C, D);

	free(M);
	M = NULL;
	free(filledData);
	filledData = NULL;
}