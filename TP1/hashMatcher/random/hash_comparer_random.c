#include <omp.h>
#include <openssl/sha.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_WORDS 1000000
#define WORD_SIZE 100
#define MAX_THREADS omp_get_max_threads()
#define MAX_FILENAME_SIZE 100

FILE *file;
char filename[MAX_FILENAME_SIZE] = "passwords_found.txt";
int stop_flag = 0;

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
int compareHashes(char **words, User *users, int num_words);
void printPasswordInfo(char *combined, User *users, int num_user, int i, int j,
                       int k, int num_words);
void HashWordSHA256(const char *input, char outputBuffer[65]);
void handleSignal(int sig);

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

int compareHashes(char **words, User *users, int num_words) {
  file = fopen(filename, "w");
  if (file == NULL) {
    printf("Could not open file %s\n", filename);
    return FILE_OPEN_ERROR;
  }

  // Handle SIGINT signal (Ctrl + C)
  signal(SIGINT, handleSignal);

  srand(time(NULL));

  printf("Starting password generation...\n");
#pragma omp parallel
  {
    while (!stop_flag) {
      int i = rand() % num_words;
      int j = rand() % num_words;
      int k = rand() % num_words;
      // printf("i: %d, j: %d, k: %d\n", i, j, k);
      char output[65];
      char combined[WORD_SIZE * 3];
      strcpy(combined, words[i]);
      strcat(combined, words[j]);
      strcat(combined, words[k]);
      HashWordSHA256(combined, output);
      int num_user = findMatchingHash(output, users);
      if (num_user != -1) {
#pragma omp critical
        {
          printPasswordInfo(combined, users, num_user, num_words, i, j, k);
          fprintf(file, "%s: %s\n", users[num_user].username, combined);
          fflush(file);
        }
      }
    }
  }

  printf("Password generation stopped.\n");
  fclose(file);
  return EXIT_SUCCESS;
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
    stop_flag = 1;
    if (file != NULL) {
      fclose(file);
    }
    // TODO: free memory
    printf("Execution stopped successfully!\n");
    exit(0);
  }
}