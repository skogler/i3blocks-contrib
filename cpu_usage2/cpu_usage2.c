// Licensed under the terms of the GNU GPL v3, or any later version.
//
// Copyright 2019 Nolan Leake <nolan@sigbus.net>
//
// Loosely based on bandwidth2 (originally by Guillaume Coré <fridim@onfi.re>)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdint.h>
#include <inttypes.h>

#define RED "#FF7373"
#define ORANGE "#FFA500"

typedef unsigned long long int ulli;
void display(const char *label, double used,
             int const warning, int const critical)
{
  if (critical != 0 && used > critical) {
    printf("{\"full_text\": \"%3.0lf%%\", \"color\": \"" RED "\"}\n", used);
  } else if (warning != 0 && used > warning) {
    printf("{\"full_text\": \"%3.0lf%%\", \"color\": \"" ORANGE "\"}\n", used);
  } else {
    printf("{\"full_text\": \"%3.0lf%%\"}\n", used);
  }
}

ulli get_usage(ulli *used_jiffies)
{
  FILE *fd = fopen("/proc/stat", "r");
  ulli user, nice, sys, idle, iowait, irq, sirq, steal, guest, nguest;

  if (!fd) {
    perror("Couldn't open /proc/stat\n");
    exit(EXIT_FAILURE);
  }
  if (fscanf(fd, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
        &user, &nice, &sys, &idle, &iowait, &irq, &sirq,
        &steal, &guest, &nguest) != 10) {
    perror("Couldn't read jiffies from /proc/stat\n");
    exit(EXIT_FAILURE);
  }
  fclose(fd);

  *used_jiffies = user + nice + sys + irq + sirq + steal + guest + nguest;
  return *used_jiffies + idle + iowait;
}

int main(int argc, char *argv[])
{
  int warning = 50, critical = 80, t = 3;

  const char *label = "CPU";

  ulli old_total;
  ulli old_used;

  old_total = get_usage(&old_used);
  sleep(1);

  while (1) {
    ulli used;
    ulli total;

    total = get_usage(&used);

    display(label, 100.0D * (used - old_used) / (total - old_total),
            warning, critical);
    fflush(stdout);
    old_total = total;
    old_used = used;

    sleep(t);
  }

  return EXIT_SUCCESS;
}
