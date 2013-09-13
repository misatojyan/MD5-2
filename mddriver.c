/* MDDRIVER.C - test driver for MD2, MD4 and MD5
 */

/* Copyright (C) 1990-2, RSA Data Security, Inc. Created 1990. All
rights reserved.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

/* The following makes MD default to MD5 if it has not already been
  defined with C compiler flags.
 */
#ifndef MD
#define MD 5
#endif

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "global.h"
#include "md5.h"


/* Length of test block, number of test blocks.
 */
#define TEST_BLOCK_LEN 1000
#define TEST_BLOCK_COUNT 1000
#define WORD_LENGTH 4

static void MDString(char *);
static void MDTimeTrial(void);
static void MDTestSuite(void);
static void MDFile(char *);
static void MDFilter(void);
static void MDPrint(unsigned char [16]);
static void MDCrypt(unsigned char [16]);
static void MDWordTime(void);

#if MD == 5
#define MD_CTX MD5_CTX
#define MDInit MD5Init
#define MDUpdate MD5Update
#define MDFinal MD5Final
#endif

/* Main driver.

Arguments (may be any combination):
  -sstring - digests string
  -t       - runs time trial
  -x       - runs test script
  filename - digests file
  (none)   - digests standard input
 */
int main (int argc, char *argv[])
{
  if (argc > 1) {
    if (strcmp(argv[1], "-s") == 0) {
      if (argc == 3)
        MDString(argv[2]);
      else
        MDString("");
    } else if (strcmp(argv[1], "-t") == 0)
      //MDTimeTrial();
      MDWordTime();
    else if (strcmp(argv[1], "-x") == 0)
      MDTestSuite();
    else if (strcmp(argv[1], "-c") == 0)
      MDCrypt("8f14e45fceea167a5a36dedd4bea2543");
    else
      MDFile(argv[1]);    
  } else
    MDFilter();
  

  return 0;
}

/* Digests a string and prints the result.
 */
static void MDString(char *string)
{
  MD_CTX context;
  unsigned char digest[16];
  unsigned int len = strlen(string);

  MDInit(&context);
  MDUpdate(&context, string, len);
  MDFinal(digest, &context);

  printf("MD%d (\"%s\") = ", MD, string);
  MDPrint(digest);
  printf("\n");
}

/* Measures the time to digest TEST_BLOCK_COUNT TEST_BLOCK_LEN-byte
  blocks.
 */
static void MDTimeTrial ()
{
  MD_CTX context;
  struct timeval endTime, startTime;
  double timedif;
  unsigned char block[TEST_BLOCK_LEN], digest[16];
  unsigned int i;
  printf("MD%d time trial. Digesting %d %d-byte blocks ...", MD, TEST_BLOCK_LEN, TEST_BLOCK_COUNT);

  /* Initialize block */
  for (i = 0; i < TEST_BLOCK_LEN; i++)
    block[i] = (unsigned char)(i & 0xff);

  /* Start timer */
  gettimeofday(&startTime, NULL);

  /* Digest blocks */
  MDInit (&context);
  for (i = 0; i < TEST_BLOCK_COUNT; i++)
    MDUpdate(&context, block, TEST_BLOCK_LEN);
  MDFinal(digest, &context);

  /* Stop timer */
  gettimeofday(&endTime, NULL);

  printf(" done\n");
  printf("Digest = ");
  MDPrint(digest);

  timedif = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) / 1000000.0;

  printf("\nTime = %f seconds\n", timedif);
  printf("Speed = %f bytes/second\n", (long)TEST_BLOCK_LEN * (long)TEST_BLOCK_COUNT / timedif);
}

/* Digests a reference suite of strings and prints the results.
 */
static void MDTestSuite ()
{
  printf("MD%d test suite:\n", MD);

  MDString("");
  MDString("a");
  MDString("abc");
  MDString("message digest");
  MDString("abcdefghijklmnopqrstuvwxyz");
  MDString("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
  MDString("1234567890123456789012345678901234567890\
            1234567890123456789012345678901234567890");
}

/* Digests a file and prints the result.
 */
static void MDFile (char *filename)
{
  FILE *file;
  MD_CTX context;
  int len;
  unsigned char buffer[1024], digest[16];

  if ((file = fopen(filename, "rb")) == NULL)
    printf ("%s can't be opened\n", filename);
  else {
    MDInit (&context);
    while (len = fread(buffer, 1, 1024, file))
      MDUpdate (&context, buffer, len);
    MDFinal(digest, &context);

    fclose(file);

    printf("MD%d (%s) = ", MD, filename);
    MDPrint(digest);
    printf("\n");
  }
}

/* Digests the standard input and prints the result.
 */
static void MDFilter ()
{
  MD_CTX context;
  int len;
  unsigned char buffer[16], digest[16];

  MDInit(&context);
  while (len = fread(buffer, 1, 16, stdin))
    MDUpdate(&context, buffer, len);
  MDFinal(digest, &context);

  MDPrint(digest);
  printf("\n");
}

/* Prints a message digest in hexadecimal.
 */
static void MDPrint(unsigned char digest[16])
{
  unsigned int i;
  for (i = 0; i < 16; i++)
    printf("%02x", digest[i]);
}

// MD5-Crypt: Find the original word
static unsigned char character[63] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
static int word_count = 0;
static unsigned char md5_target16[16];
static int found = 0;
static unsigned char original_word[64];

static void CrackWord(unsigned char word[WORD_LENGTH], int index, int len)
{
  MD_CTX context;
  unsigned char digest[16];
  unsigned int i;
  
  if (index == 0) {
    // MD5 calculate
    MDInit (&context);
    MDUpdate(&context, word, len);
    MDFinal(digest, &context);
    // MD5 finished

    for (i = 0; i < len; i++)
      printf("%c", word[i]);
    printf(":");
    MDPrint(digest);
    printf("\n");

    if (strncmp(digest, md5_target16, 16) == 0) {
      printf("\nFind!!!\n\n");
      found = 1;
      strncpy(original_word, word, len);
    }

    word_count++;
  } else {
    for (i = 0; i < 62; i++) {
      word[index - 1] = character[i];
      CrackWord(word, index - 1, len);
    }
  }
}

static void MD32ToChar16(unsigned char md5_target[32], unsigned char result[16])
{
  unsigned int i;
  int re = 0;
  for (i = 0; i < 32; i++) {
    if (md5_target[i] >= 'a' && md5_target[i] <= 'f')
      re += md5_target[i] - 'a' + 10;
    else if (md5_target[i] >= 'A' && md5_target[i] <= 'F')
      re += md5_target[i] - 'A' + 10;
    else
      re += md5_target[i] - '0';
    if (i % 2 == 0)
      re *= 16;
    else {
      result[i / 2] = re;
      re = 0;
    }
  }
}

static void MDCrypt(unsigned char md5_target[32])
{
  MD_CTX context;
  struct timeval endTime, startTime;
  double timedif;
  
  unsigned int i;
  unsigned char word[WORD_LENGTH];

  MD32ToChar16(md5_target, md5_target16);
  printf("Target MD5:\n");
  MDPrint(md5_target16);
  printf("\n");

  /* Start timer */
  gettimeofday(&startTime, NULL);

  for (i = 1; i <= WORD_LENGTH; i++)
    CrackWord(word, i, i);

  /* Stop timer */
  gettimeofday(&endTime, NULL);

  if (found)
    printf("\n\n the original word: %s\n", original_word);

  printf(" word_count: %d\n", word_count);
  printf(" done\n");

  timedif = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) / 1000000.0;
  printf("\nTime = %f seconds\n", timedif);

  return;
}

// MD5 Word Count Time Test
static void CrackWordTime(unsigned char word[WORD_LENGTH], int index, int len)
{
  MD_CTX context;
  unsigned char digest[16];
  unsigned int i;
  
  if (index == 0) {
    // MD5 calculate
    MDInit (&context);
    MDUpdate(&context, word, len);
    MDFinal(digest, &context);
    // MD5 finished

    word_count++;
  } else {
    for (i = 0; i < 62; i++) {
      word[index - 1] = character[i];
      CrackWordTime(word, index - 1, len);
    }
  }
}

static void MDWordTime()
{
  MD_CTX context;
  struct timeval endTime, startTime;
  double timedif;
  
  unsigned int i;
  unsigned char word[WORD_LENGTH];

  /* Start timer */
  gettimeofday(&startTime, NULL);

  for (i = 1; i <= WORD_LENGTH; i++)
    CrackWordTime(word, i, i);

  /* Stop timer */
  gettimeofday(&endTime, NULL);

  printf(" word_count: %d\n", word_count);
  printf(" done\n");

  timedif = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec) / 1000000.0;
  printf("\n Time = %f seconds\n", timedif);

  return;
}