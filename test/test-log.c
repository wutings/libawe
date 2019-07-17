/*
 * test-log.c
 */

#include <stdio.h>

#define AWE_LOG_TAG "test-log"
#include "awe/log.h"

#include "awe/awe.h"
#include "awe/scheduler.h"


awe_taskid tasktoken = NULL;
void test_taskproc(void* userdata, long arg){
	awe_scheduler_t *sched = (awe_scheduler_t*)userdata;
	ALOGV("test_taskproc");

	awe_scheduler_task_again(sched, tasktoken, 1000, 0);
}
void test_taskproc2(void* userdata, long arg){
	ALOGV("test_taskproc2");
}

static void stdin_cb(void* userdata, int fd, int mask){
	char buf[8] = {0};
	int r = read(fd, buf, sizeof(buf));
	ALOGV("read, %d:%s", r, buf);

	awe_scheduler_t *sched = (awe_scheduler_t*)userdata;
	if (buf[0] == 'q'){
		awe_scheduler_break(sched);
	}else if(buf[0] == 't'){
//		tasktoken = awe_scheduler_delayedtask(sched, 0, test_taskproc, sched, 10);
//		tasktoken = awe_scheduler_periodictask(sched, 0, test_taskproc, sched, 10);

		awe_scheduler_delayedtask(sched, 0, test_taskproc2, sched, 0);
		awe_scheduler_delayedtask(sched, 0, test_taskproc2, sched, 0);
	}
}

void scheduler_test(){
	awe_scheduler_t *sched = awe_scheduler_create(0);

	int rs = awe_scheduler_start(sched, "ev_sched", 0);
	ALOGV("awe_scheduler_start:%d", rs);

	awe_watchid stdin_token = awe_scheduler_watch(sched,
			STDIN_FILENO, AWE_READ, stdin_cb, sched);

	awe_scheduler_run(sched);
	awe_scheduler_socktoken_destroy(sched, &stdin_token);

	awe_scheduler_stop(sched);
	awe_scheduler_destroy(sched);
}

static void my_log_callback(int prio, const char* log_ts, const char* tag, const char* log){
	printf("my_log_callback, %s[%s]%s\n", log_ts, tag, log);
}

int main(int argc, char* argv[]){
	awe_log_init(ANDROID_LOG_DEFAULT, NULL, my_log_callback);

	scheduler_test();

	awe_log_deinit();
	ALOGV("end");

	return 0;
}
