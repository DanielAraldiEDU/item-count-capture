#define SOCKET_PATH "/tmp/pipeso"

struct Node
{
  double value;
  struct Node *next;
} node_default = {0, NULL};

struct List
{
  int length;
  double total_weight;
  struct Node *start;
  struct Node *end;
} list_default = {0, 0, NULL, NULL};

void list_delete();
void list_insert(double value);
void list_print();

void *conveyor_belt_to_bigger_weight(void *param);
void *conveyor_belt_to_medium_weight(void *param);
void *conveyor_belt_to_smaller_weight(void *param);