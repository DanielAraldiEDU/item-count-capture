#define SOCKET_PATH "/tmp/pipeso"

int global_counter = 0;
int array_length = 0;
double array[1500];
double total_weight = 0;

void array_insert(double value);
void array_print();
void array_total_weight_sum();

void *conveyor_belt_to_bigger_weight(void *param);
void *conveyor_belt_to_medium_weight(void *param);
void *conveyor_belt_to_smaller_weight(void *param);
void *send_data_to_display(void *param);