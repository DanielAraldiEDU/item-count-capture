#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

// Pipe - Socket
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

// Import main headers
#include "./main.h"

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

void array_insert(double value)
{
  if (global_counter % 1500 == 0)
  {
    array_length = 0;
  }

  array[array_length] = value;
  array_length++;
}

void array_total_weight_sum()
{
  double sum = 0;
  for (int index = 0; index < array_length; index++)
  {
    sum += array[index];
  }
  total_weight += sum;
}

void array_print()
{
  for (int index = 0; index < array_length; index++)
    printf("%f\n", array[index]);
}

void *conveyor_belt_to_bigger_weight(void *param)
{
  while (1)
  {
    pthread_mutex_lock(&count_mutex);
    if (global_counter != 0 && global_counter % 1500 == 0)
    {
      array_total_weight_sum();
      pthread_mutex_unlock(&count_mutex);
      continue;
    }

    // sleep for 1 second.
    // usleep(1000000);
    global_counter++;
    array_insert(5);

    pthread_mutex_unlock(&count_mutex);
  }

  pthread_exit(0);
}

void *conveyor_belt_to_medium_weight(void *param)
{
  while (1)
  {
    pthread_mutex_lock(&count_mutex);
    if (global_counter != 0 && global_counter % 1500 == 0)
    {
      array_total_weight_sum();
      pthread_mutex_unlock(&count_mutex);
      continue;
    }

    // sleep for 0.5 seconds.
    // usleep(500000);
    global_counter++;
    array_insert(2);

    pthread_mutex_unlock(&count_mutex);
  }

  pthread_exit(0);
}

void *conveyor_belt_to_smaller_weight(void *param)
{
  while (1)
  {
    pthread_mutex_lock(&count_mutex);
    if (global_counter != 0 && global_counter % 1500 == 0)
    {
      array_total_weight_sum();
      pthread_mutex_unlock(&count_mutex);
      continue;
    }

    // sleep for 0.1 seconds.
    // usleep(100000);
    global_counter++;
    array_insert(0.5);

    pthread_mutex_unlock(&count_mutex);
  }

  pthread_exit(0);
}

void *send_data_to_display(void *param)
{
  int server, client, length;
  struct sockaddr_un local, remote;
  char buffer[1024];

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

  while (1)
  {
    usleep(2000000);
    sprintf(buffer, "Quantidade de produtos: %d\nPeso total: %f\n", global_counter, total_weight);
    if (write(client, buffer, strlen(buffer) + 1) < 0)
    {
      perror("Write socket failed!");
      close(client);
      close(server);
      return 0;
    }
  }

  // Close sockets and exit
  close(client);
  close(server);
}

void stop_conveyor_belts()
{
  printf("Stopping conveyor belt...\n");
  int mutex_status = pthread_mutex_trylock(&count_mutex);
  if (mutex_status < 0)
  {
    pthread_mutex_unlock(&count_mutex);
  }
}

int main()
{
  signal(SIGINT, stop_conveyor_belts);
  pthread_t tid_smaller, tid_medium, tid_bigger, tid_display;
  pthread_attr_t attr;

  pthread_attr_init(&attr);

  pthread_create(&tid_display, &attr, send_data_to_display, NULL);
  pthread_create(&tid_smaller, &attr, conveyor_belt_to_smaller_weight, NULL);
  pthread_create(&tid_medium, &attr, conveyor_belt_to_medium_weight, NULL);
  pthread_create(&tid_bigger, &attr, conveyor_belt_to_bigger_weight, NULL);

  pthread_join(tid_display, NULL);
  pthread_join(tid_smaller, NULL);
  pthread_join(tid_medium, NULL);
  pthread_join(tid_bigger, NULL);

  return 0;
}