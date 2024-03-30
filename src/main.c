#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#include "./main.h"

int global_counter = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

struct List list = {0, 0, NULL, NULL};

void list_delete()
{
  struct Node *current_node = list.start;
  struct Node *next_node;

  while (current_node != NULL)
  {
    next_node = current_node->next;
    free(current_node);
    current_node = next_node;
  }
}

void list_insert(double value)
{
  list.length += 1;
  list.total_weight += value;

  struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));
  new_node->value = value;
  new_node->next = NULL;

  if (list.end == NULL)
  {
    list.start = new_node;
    list.end = new_node;
  }
  else
  {
    list.end->next = new_node;
    list.end = new_node;
  }

  printf("length:%d\n", list.length);
}

void list_print()
{
  struct Node *current_node = list.start;
  while (current_node != NULL)
  {
    printf("%f\n", current_node->value);
    current_node = current_node->next;
  }
}

void *conveyor_belt_to_bigger_weight(void *param)
{
  while (1)
  {
    if (global_counter >= 1500)
      break;
    pthread_mutex_lock(&count_mutex);

    // sleep for 1 second.
    // usleep(1000000);
    global_counter++;
    list_insert(5);

    pthread_mutex_unlock(&count_mutex);
  }

  pthread_exit(0);
}

void *conveyor_belt_to_medium_weight(void *param)
{
  while (1)
  {
    if (global_counter >= 1500)
      break;
    pthread_mutex_lock(&count_mutex);
    // sleep for 0.5 seconds.
    // usleep(500000);
    global_counter++;
    list_insert(2);

    pthread_mutex_unlock(&count_mutex);
  }

  pthread_exit(0);
}

void *conveyor_belt_to_smaller_weight(void *param)
{
  while (1)
  {
    if (global_counter >= 1500)
      break;
    pthread_mutex_lock(&count_mutex);
    // sleep for 0.1 seconds.
    // usleep(100000);
    global_counter++;
    list_insert(0.5);

    pthread_mutex_unlock(&count_mutex);
  }

  pthread_exit(0);
}

int main()
{
  int server, client, length;
  struct sockaddr_un local, remote;
  char buffer[1024];
  pthread_t tid_smaller, tid_medium, tid_bigger;
  pthread_attr_t attr;

  pthread_attr_init(&attr);

  // Create socket
  server = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server < 0)
  {
    perror("Create pipe failed!");
    return 0;
  }

  // Bind socket to local address
  memset(&local, 0, sizeof(local));
  local.sun_family = AF_UNIX;
  strncpy(local.sun_path, SOCKET_PATH, sizeof(local.sun_path) - 1);
  unlink(local.sun_path);
  length = strlen(local.sun_path) + sizeof(local.sun_family);
  if (bind(server, (struct sockaddr *)&local, length) < 0)
  {
    perror("Capture socket failed!");
    close(server);
    return 0;
  }

  if (listen(server, 5) < 0)
  {
    perror("Listen server failed!");
    close(server);
    return 0;
  }

  printf("Named pipe server listening in %s...\n", SOCKET_PATH);

  // Accept connections
  memset(&remote, 0, sizeof(remote));
  length = sizeof(remote);
  client = accept(server, (struct sockaddr *)&remote, &length);
  if (client < 0)
  {
    perror("Accept connection failed!");
    close(server);
    return 0;
  }

  printf("Client connected!\n");

  if (read(client, buffer, sizeof(buffer)) < 0)
  {
    perror("Read socket failed!");
    close(client);
    close(server);
    return 0;
  }

  printf("Received data: %s\n", buffer);

  if (write(client, buffer, strlen(buffer) + 1) < 0)
  {
    perror("Write socket failed!");
    close(client);
    close(server);
    return 0;
  }

  printf("Data sent to back to the client.\n");

  // Close sockets and exit
  close(client);
  close(server);

  pthread_create(&tid_smaller, &attr, conveyor_belt_to_smaller_weight, NULL);
  pthread_join(tid_smaller, NULL);
  pthread_create(&tid_medium, &attr, conveyor_belt_to_medium_weight, NULL);
  pthread_join(tid_medium, NULL);
  pthread_create(&tid_bigger, &attr, conveyor_belt_to_bigger_weight, NULL);
  pthread_join(tid_bigger, NULL);

  list_delete();

  return 0;
}