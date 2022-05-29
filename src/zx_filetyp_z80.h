#pragma once
#include "Z80.h"

void ZX_ReadFromFlash_SNA(Z80 *R, const char * filename);
void ZX_ReadFromFlash_Z80(Z80 *R, const uint8_t *data, uint16_t length);

int fileOpen(const char * filepath, const char * mode);
int fileRead(void * buf, int size, int handler);
int fileGetc(int handler);
int fileSeek(int handler, int seek, int origin);
int fileTell(int handler);
void fileClose(int handler);
