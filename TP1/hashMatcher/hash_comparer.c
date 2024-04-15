#include <omp.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_WORDS 1000000
#define WORD_SIZE 100
#define MAX_THREADS omp_get_max_threads()
#define MAX_FILENAME_SIZE 100

enum errors {
  WORDS_ARRAY_ALLOCATION_ERROR = 2,
  WORD_ALLOCATION_ERROR,
  FILE_OPEN_ERROR,
  HASH_READ_ERROR
};

typedef struct {
  char username[100];
  char hash[65];
  int num_users;
} User;

int readWordsFromFile(char ***words_ptr, int *num_words_ptr);
int readCSV(const char *filename, User *users);
void freeWords(char **words, int num_words);
int findMatchingHash(char *hash, User *users);
int compareHashes(char **words, User *users, int num_words);
void HashWordSHA256(const char *input, char outputBuffer[65]);

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

  compareHashes(words, users, num_words);
  freeWords(words, num_words);
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

int compareHashes(char **words, User *users, int num_words) {
  FILE *file;
  char filename[MAX_FILENAME_SIZE] = "passwords_found.txt";

  file = fopen(filename, "w");
  if (file == NULL) {
    printf("Could not open file %s\n", filename);
    return FILE_OPEN_ERROR;
  }

  //? Schedule static by default, try dynamic later

  // Hash word by word and compare with users hashes
#pragma omp parallel for num_threads(MAX_THREADS)
  for (int i = 0; i < num_words; ++i) {
    char output[65];
    HashWordSHA256(words[i], output);
    int num_user = findMatchingHash(output, users);
    if (num_user != -1) {
      printf("Se encontro la contraseña %s para el usuario %s\n", words[i],
             users[num_user].username);
      fprintf(file, "%s: %s\n", users[num_user].username, words[i]);
    }
  }

  // Hash combinations of two words and compare with users hashes
#pragma omp parallel for num_threads(MAX_THREADS)
  for (int i = 0; i < num_words; ++i) {
    for (int j = 0; j < num_words; ++j) {
      char output[65];
      char combined[WORD_SIZE * 2];
      strcpy(combined, words[i]);
      strcat(combined, words[j]);
      HashWordSHA256(combined, output);
      int num_user = findMatchingHash(output, users);
      if (num_user != -1) {
        printf("Se encontro la contraseña %s para el usuario %s\n", combined,
               users[num_user].username);
        fprintf(file, "%s: %s\n", users[num_user].username, combined);
      }
    }
  }

  // Hash combinations of three words and compare with users hashes
#pragma omp parallel for num_threads(MAX_THREADS)
  for (int i = 0; i < 10 /*num_words*/; ++i) {
    for (int j = 0; j < 10 /*num_words*/; ++j) {
      for (int k = 0; k < 10 /*num_words*/; ++k) {
        char output[65];
        char combined[WORD_SIZE * 3];
        strcpy(combined, words[i]);
        strcat(combined, words[j]);
        strcat(combined, words[k]);
        HashWordSHA256(combined, output);
        int num_user = findMatchingHash(output, users);
        if (num_user != -1) {
          printf("Se encontro la contraseña %s para el usuario %s\n", combined,
                 users[num_user].username);
          fprintf(file, "%s: %s\n", users[num_user].username, combined);
        }
      }
    }
  }
  fclose(file);
  return 0;
}

int findMatchingHash(char *hash, User *users) {
  for (int num_user = 0; num_user < users->num_users; ++num_user) {
    // printf("trying to match %s with %s\n", hash, users[i].hash);
    if (strcmp(hash, users[num_user].hash) == 0) {
      return num_user;
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
  users->num_users = num_user;
  fclose(file);
  return EXIT_SUCCESS;
}

void HashWordSHA256(const char *input, char outputBuffer[65]) {
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