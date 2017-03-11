#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <wiringPi.h>

#include "./cjson/cJSON.h"

/**
 * weight.c
 *
 * Reads data from a four load-cell scale
 * via HX711 (https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf)
 * Stores weight to a profile
 * representing the current user, in JSON.
 *
 * Sean Scott
 */

#define DEBOUNCE_TIME 500
#define MAX_USERS     4
#define MAX_LOGS      1000
#define F_JSON        "server.json"
#define PROFILE_PIN   0
#define SYNC_PIN      1
#define WEIGHT_PIN    2
#define CLOCK_PIN     5
#define DATA_PIN      4
#define SAMPLES       10
#define SPREAD        0.1
#define SCALE         10800.00

struct log
{
  int   id;
  int   weight;
  char* time;
};

struct profile
{
  int    id;
  char*  name;
  int    height;
  int    age;
  char*  gender;
  struct log* logs;
  int    numLogs;
};

volatile int currentUser = -1; // guest by default
volatile int offset = 0;       // scale offset for calculation
long lastTime = 0;             // mark for debounce

struct profile users[MAX_USERS];

/**
 * Prints current user and speaks via TTS (eSpeak).
 */
void displayCurrentUser()
{
  //printf("-----displayCurrentUser()-----\n");
  char command[128];

  if (currentUser == -1)
  {
    printf("Current User: Guest\n");
    snprintf(command, sizeof(command), "sudo espeak \"Guest\"");
  } else {
    printf("Current User (ID:%d): %s\tProfile %i\n", users[currentUser].id, users[currentUser].name, currentUser+1);

    snprintf(command, sizeof(command), "sudo espeak \"Profile %i: %s\"", currentUser+1, users[currentUser].name);
  }

  system(command);
}

/**
 * Stores all current users and corresponding logs as JSON.
 */
void saveToJson()
{
  //printf("-----saveToJson()-----\n");

  int i, j;
  cJSON * root = cJSON_CreateArray();

  for (i = 0; i < MAX_USERS; i++)
  {
    if (users[i].name)
    {
      cJSON * user;
      struct profile prof = users[i];

      cJSON_AddItemToArray(root, user = cJSON_CreateObject());
      cJSON_AddNumberToObject(user, "id", prof.id);
      cJSON_AddStringToObject(user, "name", prof.name);
      cJSON_AddNumberToObject(user, "height", prof.height);
      cJSON_AddNumberToObject(user, "age", prof.age);
      cJSON_AddStringToObject(user, "gender", prof.gender);

      cJSON * logs = cJSON_CreateArray();
      for (j = 0; j < prof.numLogs; j++)
      {
        cJSON * log;
        struct log lg = prof.logs[j];

        cJSON_AddItemToArray(logs, log = cJSON_CreateObject());
        cJSON_AddNumberToObject(log, "id", lg.id);
        cJSON_AddNumberToObject(log, "weight", lg.weight);
        cJSON_AddStringToObject(log, "time", lg.time);
      }
      cJSON_AddItemToObject(user, "logs", logs);
    }
  }

  char * rendered = cJSON_Print(root);

  FILE *fp = fopen(F_JSON, "w");
  int res = fputs(rendered, fp);
  if (res == EOF)
  {
    fprintf(stderr, "Unable to save: %s\n", strerror(errno));
  } else {
    printf("Success saving %s\n", F_JSON);
  }

  fclose(fp);
}

/**
 * Appends current user's weight log with new entry.
 */
void addLog(int weight, char * time)
{
  //printf("-----addLog()-----\n");

  if (users[currentUser].name)
  {
    // Create ID by adding one to last log
    int lastLogIndex = users[currentUser].numLogs - 1;
    struct log lastLog = users[currentUser].logs[lastLogIndex];
    int id = lastLog.id + 1;

    // Make new log
    struct log nLog;
    nLog.id = id;
    nLog.weight = weight;
    nLog.time = time;

    // Add log to user
    users[currentUser].logs[lastLogIndex + 1] = nLog;
    users[currentUser].numLogs++;

    saveToJson();
  }
}

/**
 * Cycles through all existing users and guest.
 */
void changeUser (void)
{
  //printf("-----changeUser()-----\n");

  // Debounce
  if (millis() - lastTime > DEBOUNCE_TIME)
  {
    if (!users[++currentUser].name)
    {
      currentUser = -1;
    }

    displayCurrentUser();
  }

  lastTime = millis();

}

/**
 * Parses JSON file.
 */
void readFile(char *buf)
{
  //printf("-----readFile()-----\n");

  int i;

  cJSON * root = cJSON_Parse(buf);
  for (i = 0; i < MAX_USERS; i++)
  {
    cJSON * user = cJSON_GetArrayItem(root, i);
    if (user)
    {
      int id = cJSON_GetObjectItem(user, "id")->valueint;
      char * name = cJSON_GetObjectItem(user, "name")->valuestring;
      int height = cJSON_GetObjectItem(user, "height")->valueint;
      int age = cJSON_GetObjectItem(user, "age")->valueint;
      char * gender = cJSON_GetObjectItem(user, "gender")->valuestring;

      cJSON * jsonLogs = cJSON_GetObjectItem(user, "logs");
      struct log logs[MAX_LOGS];

      int finished = 0;
      int j = 0;
      while(finished == 0)
      {
        cJSON * jsonLog = cJSON_GetArrayItem(jsonLogs, j);
        if (jsonLog)
        {
          int id = cJSON_GetObjectItem(jsonLog, "id")->valueint;
          int weight = cJSON_GetObjectItem(jsonLog, "weight")->valueint;
          char * time = cJSON_GetObjectItem(jsonLog, "time")->valuestring;

          struct log aLog;
          aLog.id = id;
          aLog.weight = weight;
          aLog.time = time;

          logs[j] = aLog;
          j++;
        } else {
          finished = 1;
        }
      }

      struct profile user;
      user.id = id;
      user.name = name;
      user.height = height;
      user.age = age;
      user.gender = gender;
      user.logs = logs;
      user.numLogs = j;

      users[i] = user;

      //printf("\tid:\t\t%d\n", user.id);
      //printf("\tname:\t\t%s\n", user.name);
      //printf("\theight:\t\t%d\n", user.height);
      //printf("\tage:\t\t%d\n", user.age);
      //printf("\tgender:\t\t%s\n", user.gender);
      //printf("\tnumLogs:\t\t%d\n", user.numLogs);
    }
  }
}

/**
 * Loads JSON file into buffer.
 */
void loadFile(char* filename)
{
  //printf("-----loadFile()-----\n");

  FILE *fp;
  long lSize;
  char *buffer;

  fp = fopen(filename, "rb");
  if (!fp)
  {
    perror(filename);
    exit(1);
  }

  fseek(fp, 0L, SEEK_END);
  lSize = ftell(fp);
  rewind(fp);

  // allocate memory
  buffer = calloc(1, lSize + 1);
  if (!buffer)
  {
    fclose(fp);
    fputs("memory alloc", stderr);
    exit(1);
  }

  // copy file into buffer
  if (1 != fread(buffer, lSize, 1, fp))
  {
    fclose(fp);
    free(buffer);
    fputs("read", stderr);
    exit(1);
  }

  fclose(fp);

  readFile(buffer);

  free(buffer);
}

/**
 * Reloads profiles file and repopulates the profile and log arrays.
 */
void refreshData(void)
{
  if (millis() - lastTime > DEBOUNCE_TIME)
  {
    currentUser = -1;
    loadFile(F_JSON);
  }

  lastTime = millis();
}

void powerUp()
{
  digitalWrite(CLOCK_PIN, LOW);
  delayMicroseconds(1);
}

void powerDown()
{
  digitalWrite(CLOCK_PIN, HIGH);
  delayMicroseconds(1);
}

/**
 * Reads raw value from HX711
 * From Datasheet - "Reference Driver (C)"
 */
unsigned long getScaleData(int setup)
{
  //printf("-----getScaleData()-----\n");

  long count;
  int i, ret;

  count = 0;

  while((uint8_t)digitalRead(DATA_PIN));

  powerUp();
  for (i = 0; i < 24; i++)
  {
    powerDown();
    count = count << 1;
    powerUp();
    if (((uint8_t)digitalRead(DATA_PIN)) > 0)
    {
      count++;
    }
  }

  powerDown();

  count = count^0x800000;

  powerUp();

  if (setup == 0)
  {
    ret = count;
  }
  else
  {
    ret = count - offset;
  }

  return ret;
}

float getWeight(int setup)
{
  float weight;
  int i, j;
  long avg = 0;
  float low, high;
  long samples[SAMPLES];

  for (i = 0; i < SAMPLES; i++)
  {
    samples[i] = getScaleData(setup);
    avg += samples[i];

    //printf("samples[%i]: %ld\n", i, samples[i]);
  }

  avg /= SAMPLES;

  //printf("\navg: %ld\n", avg);

  low = (float) avg * (1.0 - SPREAD);
  high = (float) avg * (1.0 + SPREAD);

  //printf("\nlow: %.2f\n", low);
  //printf("\nhigh: %.2f\n", high);

  avg = 0;
  j = 0;

  for (i = 0; i < SAMPLES; i++)
  {
    if ((samples[i] < high && samples[i] > low) ||
        (samples[i] > high && samples[i] < low))
    {
      avg += samples[i];
      j++;
    }
  }

  //printf("avg: %ld\n", avg);
  //printf("j: %i\n", j);

  if (setup == 0)
  {
    if (j > 0)
    {
      offset = (avg / j);
      //printf("Setup complete with offset: %i\n", offset);
    }
  }
  else
  {
    if (j > 0)
    {
      weight = (avg / j) / SCALE;
    }
  }

  return weight;
}

void takeWeight(void)
{
  if (millis() - lastTime > DEBOUNCE_TIME)
  {
    int weight = (int)getWeight(1);

    //printf("Weight: %i\n", weight);

    char command[128];

    if (currentUser == -1)
    {
      snprintf(command, sizeof(command), "sudo espeak \"Hello Guest, your weight is %i pounds\"", weight);
    } else {
      int height = users[currentUser].height;
      float bmi = (weight * 703)/(height*height);

      snprintf(command, sizeof(command), "sudo espeak \"Hello %s, your weight is %i pounds, and your body mass index is %.1f\"", users[currentUser].name, weight, bmi);

      // Get current time as char[] and add log
      time_t timer;
      char timebuf[26];
      struct tm* tm_info;

      time(&timer);
      tm_info = localtime(&timer);

      strftime(timebuf, 26, "%Y-%m-%d %H:%M:%S", tm_info);
      addLog(weight, timebuf);

    }
    // Speak!
    system(command);
  }

  lastTime = millis();
}

/**
 * Assigns input/output to GPIO pins
 */
void setupGpio()
{
  //printf("-----setupGpio()-----\n");

  // Buttons
  pinMode(PROFILE_PIN, INPUT);
  pullUpDnControl(PROFILE_PIN, PUD_UP);

  pinMode(SYNC_PIN, INPUT);
  pullUpDnControl(SYNC_PIN, PUD_UP);

  pinMode(WEIGHT_PIN, INPUT);
  pullUpDnControl(WEIGHT_PIN, PUD_UP);

  // Scale
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, INPUT);
  digitalWrite(CLOCK_PIN, LOW);

  // Setup wiringPi interrupts
  wiringPiISR(PROFILE_PIN, INT_EDGE_FALLING, &changeUser);
  wiringPiISR(SYNC_PIN, INT_EDGE_FALLING, &refreshData);
  wiringPiISR(WEIGHT_PIN, INT_EDGE_FALLING, &takeWeight);
}

int main()
{
  loadFile(F_JSON);

  // Setup wiringPi library
  if (wiringPiSetup() == -1)
  {
    fprintf(stderr, "Unable to setup wiringPi: %s\n", strerror(errno));
    exit(1);
  }

  // Setup I/O
  setupGpio();

  // Get offset value for scale
  getWeight(0);

  // Main loop
  while(1)
  {
    delay(1000);
  }

  return 0;
}
