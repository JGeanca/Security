#include <omp.h>
#include <openssl/sha.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_WORDS 1000000
#define WORD_SIZE 100
#define MAX_THREADS omp_get_max_threads()
#define MAX_FILENAME_SIZE 100

FILE *file;
char filename[MAX_FILENAME_SIZE] = "passwords_found.txt";

enum errors {
  WORDS_ARRAY_ALLOCATION_ERROR = 2,
  WORD_ALLOCATION_ERROR,
  FILE_OPEN_ERROR,
  HASH_READ_ERROR,
  NUM_ARGS_ERROR,
  INVALID_INDEXES_ERROR
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
int compareHashes(char **words, User *users, int num_words, int start_index,
                  int end_index, int stage_2);
void printPasswordInfo(char *combined, User *users, int num_user, int i, int j,
                       int k, int num_words);
void HashWordSHA256(const char *input, char outputBuffer[65]);
int getArgs(int argc, char *argv[], int *start_index, int *end_index,
            int *stage_2);
void handleSignal(int sig);

int main(int argc, char *argv[]) {
  int start_index, end_index, stage_2 = 0;
  if (getArgs(argc, argv, &start_index, &end_index, &stage_2) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }
  char **words;
  int num_words;
  User users[10];

  if (readCSV("users.csv", users) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }
  if (readWordsFromFile(&words, &num_words) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }
  compareHashes(words, users, num_words, start_index, end_index, stage_2);
  freeWords(words, num_words);
  return EXIT_SUCCESS;
}

int compareHashes(char **words, User *users, int num_words, int start_index,
                  int end_index, int stage_2) {
  file = fopen(filename, "w");
  if (file == NULL) {
    printf("Could not open file %s\n", filename);
    return FILE_OPEN_ERROR;
  }

  // Handle SIGINT signal (Ctrl + C)
  signal(SIGINT, handleSignal);

  // Hash combinations of two words and compare with users hashes
  if (stage_2) {
    printf("Starting Stage 2. st: %d, fn: %d\n", start_index, end_index);
#pragma omp parallel for num_threads(MAX_THREADS)
    for (int i = start_index; i < end_index; ++i) {
      for (int j = 0; j < num_words; ++j) {
        char output[65];
        char combined[WORD_SIZE * 2];
        strcpy(combined, words[i]);
        strcat(combined, words[j]);
        HashWordSHA256(combined, output);
        int num_user = findMatchingHash(output, users);
        if (num_user != -1) {
          printPasswordInfo(combined, users, num_user, i, j, -1, num_words);
          fprintf(file, "%s: %s\n", users[num_user].username, combined);
        }
      }
    }
  }
  printf("\nStage 2 finished\n\n");
  printf("Starting Stage 3. start: %d, end: %d\n", start_index, end_index);

  // Hash combinations of three words and compare with users hashes
#pragma omp parallel for num_threads(MAX_THREADS)
  for (int i = start_index; i < end_index; ++i) {
    for (int j = 0; j < num_words; ++j) {
      for (int k = 0; k < num_words; ++k) {
        char output[65];
        char combined[WORD_SIZE * 3];
        strcpy(combined, words[i]);
        strcat(combined, words[j]);
        strcat(combined, words[k]);
        HashWordSHA256(combined, output);
        int num_user = findMatchingHash(output, users);
        if (num_user != -1) {
          printPasswordInfo(combined, users, num_user, i, j, k, num_words);
          fprintf(file, "%s: %s -> i: %d, j: %d, k: %d \n",
                  users[num_user].username, combined, i, j, k);
        }
      }
    }
  }
  printf("\nStage 3 finished\n");
  fclose(file);
  return 0;
}

void printPasswordInfo(char *combined, User *users, int num_user, int i, int j,
                       int k, int num_words) {
  printf("The password was found: %s for the user: %s\n", combined,
         users[num_user].username);
  if (k != -1) {
    printf("Tries: %d\n", i * num_words * num_words + j * num_words + k);
  } else {
    printf("Tries: %d\n", i * num_words + j);
  }

  printf("Indexes: i: %d, j: %d, k: %d\n", i, j, k);
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
  FILE *fp = fopen(filename, "r");

  if (!fp) {
    printf("Could not open %s', filename\n", filename);
    return FILE_OPEN_ERROR;
  }

  char line[1024];
  while (fgets(line, sizeof(line), fp)) {
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
  fclose(fp);
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

int getArgs(int argc, char *argv[], int *start_index, int *end_index,
            int *stage_2) {
  if (argc != 4) {
    printf("Usage: %s <start_index> <end_index> <Stage_2? 0 : 1>\n", argv[0]);
    return NUM_ARGS_ERROR;
  }

  *start_index = atoi(argv[1]);
  *end_index = atoi(argv[2]);
  *stage_2 = atoi(argv[3]);

  if (*start_index < 0 || *end_index < 0 || *start_index > *end_index) {
    printf("Invalid indexes\n");
    return INVALID_INDEXES_ERROR;
  }

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

  FILE *fp = fopen(url, "r");
  if (fp == NULL) {
    printf("Could not open file\n");
    free(words);
    return FILE_OPEN_ERROR;
  }

  int num_words = 0;
  while (num_words < NUM_WORDS && fscanf(fp, "%s", word) == 1) {
    words[num_words] = malloc(strlen(word) + 1);
    if (words[num_words] == NULL) {
      printf("Could not allocate memory for word\n");
      freeWords(words, num_words);
      fclose(fp);
      return WORD_ALLOCATION_ERROR;
    }
    strcpy(words[num_words], word);
    ++num_words;
  }

  fclose(fp);
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

void handleSignal(int sig) {
  if (sig == SIGINT) {
    printf("\nStop signal received...\n");
    if (file != NULL) {
      fclose(file);
    }
    // TODO: free memory
    printf("Execution stopped successfully!\n");
    exit(0);
  }
}