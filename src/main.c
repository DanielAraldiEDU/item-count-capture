#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Pipe - Socket
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

// Signal
#include <signal.h>

// OpenMP
#include <omp.h>

// Import main headers
#include "./main.h"

omp_lock_t lock;

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

void conveyor_belt_to_bigger_weight()
{
  while (1)
  {
    // sleep for 1 second.
    usleep(1000000);

#pragma omp critical
    {
      omp_set_lock(&lock);

      if (!is_weight_summed && global_counter != 0 && global_counter % 1500 == 0)
      {
        is_weight_summed = 1;
        array_total_weight_sum();
      }
      else
      {
        is_weight_summed = 0;
        array_insert(5);
        global_counter++;
      }

      omp_unset_lock(&lock);
    }
  }

  omp_destroy_lock(&lock);
}

void conveyor_belt_to_medium_weight()
{
  while (1)
  {
    // sleep for 0.5 seconds.
    usleep(500000);

#pragma omp critical
    {
      omp_set_lock(&lock);

      if (!is_weight_summed && global_counter != 0 && global_counter % 1500 == 0)
      {
        is_weight_summed = 1;
        array_total_weight_sum();
      }
      else
      {
        is_weight_summed = 0;
        array_insert(2);
        global_counter++;
      }

      omp_unset_lock(&lock);
    }
  }

  omp_destroy_lock(&lock);
}

void conveyor_belt_to_smaller_weight()
{
  while (1)
  {
    // sleep for 0.1 seconds.
    usleep(100000);

#pragma omp critical
    {
      omp_set_lock(&lock);

      if (!is_weight_summed && global_counter != 0 && global_counter % 1500 == 0)
      {
        is_weight_summed = 1;
        array_total_weight_sum();
      }
      else
      {
        is_weight_summed = 0;
        array_insert(0.5);
        global_counter++;
      }

      omp_unset_lock(&lock);
    }
  }

  omp_destroy_lock(&lock);
}

void send_data_to_display()
{
  int server, client, length;
  struct sockaddr_un local, remote;
  char buffer[1024];

  // Create socket
  server = socket(AF_UNIX, SOCK_STREAM, 0);
  if (server < 0)
  {
    perror("Create pipe failed!");
    return;
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
    return;
  }

  if (listen(server, 5) < 0)
  {
    perror("Listen server failed!");
    close(server);
    return;
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
    return;
  }

  printf("Display connected!\n");

  while (1)
  {
    if (!is_stopped)
    {
      usleep(2000000);

      sprintf(buffer, "Amount of products: %d\nTotal weight: %f\n", global_counter, total_weight);
      if (write(client, buffer, strlen(buffer) + 1) < 0)
      {
        perror("Write socket failed!");
        close(client);
        close(server);
        return;
      }
    }
  }

  // Close sockets and exit
  close(client);
  close(server);
}

void stop_conveyor_belts()
{
  is_stopped = !is_stopped;
  if (is_stopped)
  {
    fprintf(stderr, "\nConveyor belt stopping!\n");
    omp_set_lock(&lock);
  }
  else
  {
    fprintf(stderr, "\nConveyor belt working!\n");
    omp_unset_lock(&lock);
  }
}

int main()
{
  signal(SIGINT, stop_conveyor_belts);

  omp_init_lock(&lock);

#pragma omp parallel shared(global_counter, total_weight, array, array_length, is_weight_summed, is_stopped)
  {
#pragma omp sections nowait
    {
#pragma omp section
      {
        conveyor_belt_to_bigger_weight();
      }
#pragma omp section
      {
        conveyor_belt_to_medium_weight();
      }
#pragma omp section
      {
        conveyor_belt_to_smaller_weight();
      }
#pragma omp section
      {
        send_data_to_display();
      }
    }
  }

  return 0;
}