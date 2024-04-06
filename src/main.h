#define SOCKET_PATH "/tmp/pipeso"

// This value is used to determine a boolean value (0 - false and 1 - true)
int is_weight_summed = 0;
int is_stopped = 0;

int global_counter = 0;
int array_length = 0;
double array[1500];
double total_weight = 0;

void array_insert(double value);
void array_total_weight_sum();

void *conveyor_belt_to_bigger_weight(void *param);
void *conveyor_belt_to_medium_weight(void *param);
void *conveyor_belt_to_smaller_weight(void *param);
void *send_data_to_display(void *param);