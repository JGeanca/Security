#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_WORDS 1000000
#define WORD_SIZE 100
#define MAX_THREADS omp_get_max_threads()

enum errors {
  WORDS_ARRAY_ALLOCATION_ERROR = 2,
  WORD_ALLOCATION_ERROR,
  FILE_OPEN_ERROR,
  HASH_READ_ERROR
};

typedef struct {
  char username[100];
  char hash[65];
} User;

int readWordsFromFile(char ***words_ptr, int *num_words_ptr);
int readCSV(const char *filename, User *users);
void freeWords(char **words, int num_words);

int main() {
  char **words;
  int num_words;

  User users[10];

  if (readCSV("users.csv", users) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  if (readWordsFromFile(&words, &num_words) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }

  for (int i = 0; i < 10; ++i) {
    printf("User: %s, Hash: %s\n", users[i].username, users[i].hash);
  }

  // for (int i = 0; i < 10; ++i) {
  //   printf("%s\n", words[i]);
  // }
  // printf("words num: %d\n", num_words);
  // printf("last word: %s\n", words[num_words - 1]);

  // compareHashes(words);

  // freeWords(words, num_words);
  return EXIT_SUCCESS;
}

int readWordsFromFile(char ***words_ptr, int *num_words_ptr) {
  const char url[] = "xato-net-10-million-passwords-1000000.txt";

  char word[WORD_SIZE];
  char **words = malloc(NUM_WORDS * sizeof(char *));
  if (words == NULL) {
    printf("Could not allocate memory for words\n");
    return WORDS_ARRAY_ALLOCATION_ERROR;
  }

  FILE *file = fopen(url, "r");
  if (file == NULL) {
    printf("Could not open file\n");
    free(words);
    return FILE_OPEN_ERROR;
  }

  int num_words = 0;
  while (num_words < NUM_WORDS && fscanf(file, "%s", word) == 1) {
    words[num_words] = malloc(strlen(word) + 1);
    if (words[num_words] == NULL) {
      printf("Could not allocate memory for word\n");
      freeWords(words, num_words);
      fclose(file);
      return WORD_ALLOCATION_ERROR;
    }
    strcpy(words[num_words], word);
    ++num_words;
  }

  fclose(file);
  *words_ptr = words;
  *num_words_ptr = num_words;

  return EXIT_SUCCESS;
}

// free words memory
void freeWords(char **words, int num_words) {
  for (int i = 0; i < num_words; ++i) {
    free(words[i]);
  }
  free(words);
  return;
}

int compareHashes(char **input) {
  //? Schedule static by default, try dinamic later
#pragma omp parallel for num_threads(MAX_THREADS)
  for (int i = 0; i < 10 /*len(input)*/; ++i) {
    char output[65];
    sha256(input[i], output);
    // int user_index = findMatchingHash(output, users, num_users);
    // if (user_index != -1) {
    //   printf("Hash encontrado para la palabra \"%s\": Usuario %s\n",
    //   input[i],
    //          users[user_index].username);
    // }
  }
  return 0;
}

void sha256(const char *input, char outputBuffer[65]) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, input, strlen(input));
  SHA256_Final(hash, &sha256);
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
  }
  outputBuffer[64] = 0;
}

int findMatchingHash(char *hash, User *users, int num_users) {
  for (int i = 0; i < num_users; i++) {
    if (strcmp(hash, users[i].hash) == 0) {
      return i;
    }
  }
  return -1;
}

int readCSV(const char *filename, User *users) {
  int num_user = 0;
  FILE *file = fopen(filename, "r");

  if (!file) {
    printf("Could not open %s', filename\n", filename);
    return FILE_OPEN_ERROR;
  }

  char line[1024];
  while (fgets(line, sizeof(line), file)) {
    char *username = strtok(line, ",");
    if (!username) {
      continue;
    }

    strcpy(users[num_user].username, username);

    char *hash = strtok(NULL, ",");
    if (!hash) {
      printf("Error: Could not read hash\n");
      return HASH_READ_ERROR;
    }

    hash[strcspn(hash, "\n")] = 0;  // Clean newline character
    strcpy(users[num_user].hash, hash);
    ++num_user;
  }
  fclose(file);
  return EXIT_SUCCESS;
}