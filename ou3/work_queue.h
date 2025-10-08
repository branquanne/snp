
typedef struct work_queue work_queue_t;
work_queue_t* work_queue_create(void);
void work_queue_push(work_queue_t* queue, const char* path);