#ifndef STARRUBY_PRIVATE_H
#define STARRUBY_PRIVATE_H

#ifndef PI
#define PI (3.1415926535897932384626433832795)
#endif

#define MAX(x, y) ((x >= y) ? x : y)
#define MIN(x, y) ((x <= y) ? x : y)
#define DIV255(x) ((x + 255) >> 8)

VALUE GetCompletePath(VALUE, bool);

void InitializeAudio(void);
void InitializeColor(void);
void InitializeGame(void);
void InitializeFont(void);
void InitializeInput(void);
void InitializeStarRubyError(void);
void InitializeTexture(void);

void UpdateAudio(void);
void UpdateInput(void);

void FinalizeAudio(void);
void FinalizeInput(void);

void InitializeSdlAudio(void);
void InitializeSdlFont(void);
void InitializeSdlInput(void);

int GetWindowScale(void);

#ifdef DEBUG
#include <assert.h>
void TestInput(void);
#endif

#endif
