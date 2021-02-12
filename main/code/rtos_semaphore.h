/*
 * rtos_semaphore.h
 *
 *  Created on: Feb 12, 2021
 *      Author: wit
 */

#ifndef MAIN_CODE_RTOS_SEMAPHORE_H_
#define MAIN_CODE_RTOS_SEMAPHORE_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class rtos_semaphore {
public:
	rtos_semaphore() {
		s = xSemaphoreCreateBinary();
		if (s == NULL) {
			ESP_LOGI("semaphore", "Failed to create");
		}
	}
	void give() {
		xSemaphoreGive(s);
	}
	bool take(int timeout = portMAX_DELAY) {
		if (xSemaphoreTake(s, timeout) == pdTRUE) {
			return true;
		} else {
			return false;
		}
	}
	bool is_ready() {
		if (uxSemaphoreGetCount(s) != 0) {
			return true;
		} else {
			return false;
		}
	}
private:
	SemaphoreHandle_t s;
};


#endif /* MAIN_CODE_RTOS_SEMAPHORE_H_ */
