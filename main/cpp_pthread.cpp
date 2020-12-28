#include "spifs.h"
#include "espfile.h"
#include "wifi.h"
#include "mqttclient.h"
#include "json.hpp"

#include <esp_log.h>
#include <esp_pthread.h>
#include <thread>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "nvs_flash.h"


#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"

#include "aws.h"

#include "onewirebus.h"
#include "ds18b20.h"

#include "esp_sleep.h"
#include <thread>
#include <map>

#include "i2cbus.h"

const std::string cfg_partition("config");
const std::string cfg_mountpoint("/config");
std::string device_ID("esp32-test-idf");
const std::string topic = std::string("$aws/things/esptest/shadow/update");

#define TAG "MAIN_APP"
#define TAG_MEASURE "MEASURE_APP"


void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data) {
    ESP_LOGW(TAG, "MQTT Disconnect");
}

void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData) {
    ESP_LOGI(TAG, "Subscribe callback");
    ESP_LOGI(TAG, "%.*s\t%.*s", topicNameLen, topicName, (int) params->payloadLen, (char *)params->payload);
}

esp_pthread_cfg_t create_config(const char *name, int core_id, int stack, int prio)
{
    auto cfg = esp_pthread_get_default_config();
    cfg.thread_name = name;
    cfg.pin_to_core = core_id;
    cfg.stack_size = stack;
    cfg.prio = prio;
    return cfg;
}

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
private:
	SemaphoreHandle_t s;
};

//void handle_onewire_sensor_bus(nlohmann::json &cfg, int bus_id, rtos_semaphore &done_sem, rtos_semaphore &data_sem) {
void handle_onewire_sensor_bus(nlohmann::json &cfg, int bus_id, rtos_semaphore &done_sem, rtos_semaphore &result_sem, nlohmann::json &results) {
	std::string bus_name = "onewire_bus_" + std::to_string(bus_id);
	int sensor_count = cfg["sensor_configuration"][bus_name]["sensor_count"];
	gpio_num_t busgpio = cfg["sensor_configuration"][bus_name]["bus_gpio"];
	rmt_channel_t rx_channel = cfg["sensor_configuration"][bus_name]["rmt_rx"];
	rmt_channel_t tx_channel = cfg["sensor_configuration"][bus_name]["rmt_tx"];

	ESP_LOGI(bus_name.c_str(), "Starting onewire thread, gpio: %d, rx: %d, tx: %d", (int)busgpio, (int)rx_channel, (int)tx_channel);

	onewire_bus b(busgpio, tx_channel, rx_channel);
	auto devices = ds18b20_factory::get_all_ds18b20_map(b);

	ESP_LOGI(bus_name.c_str(), "Found %d devices", devices.size());

	if (devices.size() != 0) {
		devices.begin()->second->all_dev_convert_temperature();
	}

	for (int i=0; i<sensor_count; i++) {
		std::string sensor_name = "sensor_" + std::to_string(i);
		std::string s = cfg["sensor_configuration"][bus_name][sensor_name]["rom_code"];

		if (devices.count(s)) {
			ESP_LOGI(bus_name.c_str(), "Sensor detected");
			auto temp = devices[s]->get_temperature();

			result_sem.take();
			int count = results["state"]["reported"]["sensors"]["sensor_count"];
			results["state"]["reported"]["sensors"][sensor_name]["meas_type"]   = cfg["sensor_configuration"][bus_name][sensor_name]["meas_type"].get<std::string>();
			results["state"]["reported"]["sensors"][sensor_name]["type"]        = cfg["sensor_configuration"][bus_name][sensor_name]["sensor_type"].get<std::string>();
			results["state"]["reported"]["sensors"][sensor_name]["name"]        = cfg["sensor_configuration"][bus_name][sensor_name]["name"].get<std::string>();
			results["state"]["reported"]["sensors"][sensor_name]["sensor_id"]        = cfg["sensor_configuration"][bus_name][sensor_name]["sensor_id"].get<int>();
			results["state"]["reported"]["sensors"][sensor_name]["value"]       = temp;
			results["state"]["reported"]["sensors"]["sensor_count"] = count + 1;
			result_sem.give();
		}
	}

	done_sem.give();
}

void start_measurements(nlohmann::json &cfg, nlohmann::json &results, rtos_semaphore &done) {
	ESP_LOGI(TAG_MEASURE, "Start measurement app");
	rtos_semaphore result_semaphore;
	result_semaphore.give();

	results["state"]["reported"]["sensors"]["sensor_count"] = 0;

	ESP_LOGI(TAG_MEASURE, "Start onewire threads");
	int onewirebus_count = cfg["sensor_configuration"]["onewire_bus_count"];
	std::vector<std::tuple<rtos_semaphore *, std::thread *>> *onewire_thread_list = new std::vector<std::tuple<rtos_semaphore *, std::thread *>>();
	for (int i=0; i<onewirebus_count; i++) {
	    auto thread_cfg = create_config("Meas", 1, 5*1024, 5);
	    esp_pthread_set_cfg(&thread_cfg);
		rtos_semaphore *sem = new rtos_semaphore();
		std::thread *t = new std::thread(handle_onewire_sensor_bus, std::ref(cfg), i, std::ref(*sem), std::ref(result_semaphore), std::ref(results));
		onewire_thread_list->push_back(std::tuple<rtos_semaphore *, std::thread *>(sem, t));
	}
	for (auto &e : *onewire_thread_list) {
		std::get<0>(e)->take();
		delete std::get<0>(e);
		std::get<1>(e)->join();
		delete std::get<1>(e);
	}
	delete onewire_thread_list;
	ESP_LOGI(TAG_MEASURE, "Onewire threads done");

	done.give();
}

extern "C" void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    spifs config_fs = spifs(cfg_partition, cfg_mountpoint);
    esp_file config = esp_file(config_fs, std::string("/ha_config.json"));

    nlohmann::json cfg = nlohmann::json::parse(config.get_file_buffer());
    nlohmann::json measurement_results;

    /* Semaphores and threads */
    rtos_semaphore measurement_done;
    auto thread_cfg = create_config("Meas", 1, 10*1024, 5);
    esp_pthread_set_cfg(&thread_cfg);
    auto t = std::thread(start_measurements, std::ref(cfg), std::ref(measurement_results), std::ref(measurement_done));

    /* Start WiFi */
    wifi w;
    w.set_creditentials(cfg["wifi"]["wifi_name"].get<std::string>(), cfg["wifi"]["wifi_pass"].get<std::string>());
    w.wifi_start_sta();

    /* Get unique device id */
    uint64_t dev_id = 0;
    esp_efuse_mac_get_default(reinterpret_cast<uint8_t *>(&dev_id));
	ESP_LOGI(TAG, "Device id: %llx", dev_id);

    /* Certs and params for AWS IoT cloud */
    esp_file aws_root_ca                 = esp_file(config_fs, cfg["aws"]["aws_ca_cert"].get<std::string>());
    esp_file aws_dev_certificate         = esp_file(config_fs, cfg["aws"]["aws_certificate"].get<std::string>());
    esp_file aws_dev_private_key         = esp_file(config_fs, cfg["aws"]["aws_private_key"].get<std::string>());
    std::string aws_endpoint             = cfg["aws"]["aws_endpoint"].get<std::string>();
    std::string aws_thing_name           = cfg["aws"]["aws_thing_name"].get<std::string>();
    std::string aws_thing_mqtt_channel   = "$aws/things/" + aws_thing_name + "/shadow/update";
    std::string aws_thing_mqtt_receive   = "$aws/things/" + aws_thing_name + "/shadow/get/accepted";
    std::string aws_thing_mqtt_request   = "$aws/things/" + aws_thing_name + "/shadow/get";


    /* Parameters for device configuration */
    std::string device_location = cfg["device"]["device_location"].get<std::string>();
    int sleep_length = cfg["device"]["sleep_length"].get<int>();
	int device_idx   = cfg["device"]["device_id"].get<int>();

    aws_connection_mqtt c = aws_connection_mqtt();
    c.endpoint_URL(aws_endpoint);
    c.root_CA(aws_root_ca);
    c.device_Cert(aws_dev_certificate);
    c.private_Key(aws_dev_private_key);
    c.client_ID(device_ID);
    c.device_name(aws_thing_name);

    w.wait_for_ready();

    if (SUCCESS != c.init_connection())
        abort();
    c.connect(3);
    measurement_done.take();

    /* Register channel */
    //aws_iot_mqtt_subscribe(&c.client, &aws_thing_mqtt_receive[0], aws_thing_mqtt_receive.length(), QOS0, iot_subscribe_callback_handler, NULL);

    /* Device IDX */
    measurement_results["state"]["reported"]["unique_id"] = dev_id;
    measurement_results["state"]["reported"]["id"] = device_idx;
    measurement_results["state"]["reported"]["sensors"]["sensor_stats"]["soc"]   	   = 35;
    measurement_results["state"]["reported"]["sensors"]["sensor_stats"]["rssid"]     = 15;
    measurement_results["state"]["reported"]["sensors"]["sensor_stats"]["location"]  = cfg["device"]["device_location"].get<std::string>();

    /* Battery RSOC and Voltage */
    i2cbus i(0, GPIO_NUM_18, GPIO_NUM_19);
    try {
    	auto v = i.i2c_read_regs(0x0B, 0x09, 2);
    	int volts = ((*v)[1] << 8) | (*v)[0];
    	ESP_LOGI(TAG, "volts: %d", volts);

		int count = measurement_results["state"]["reported"]["sensors"]["sensor_count"];
		std::string sensor_name = "sensor_" + std::to_string(count);
		measurement_results["state"]["reported"]["sensors"][sensor_name]["meas_type"]   = "battery_voltage";
		measurement_results["state"]["reported"]["sensors"][sensor_name]["type"]        = "V";
		measurement_results["state"]["reported"]["sensors"][sensor_name]["name"]        = "battery_voltage";
		measurement_results["state"]["reported"]["sensors"][sensor_name]["value"]       = volts;
		measurement_results["state"]["reported"]["sensors"][sensor_name]["sensor_id"]   = 10;
		measurement_results["state"]["reported"]["sensors"]["sensor_count"] = count + 1;
    } catch (...) {
    	ESP_LOGI(TAG, "Failed to get battery voltage");
	}

    auto s = measurement_results.dump();
    aws_publish_message msg = aws_publish_message(QOS0, reinterpret_cast<void *>(&s[0]), s.length(), aws_thing_mqtt_channel);
    c.publish_msg(msg);

    if( NETWORK_ATTEMPTING_RECONNECT == c.yeld(1)) {
    	ESP_LOGI(TAG, "reconnecting");
    }


    ESP_LOGI(TAG, "Stack remaining for task '%s' is %d bytes", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL));
    ESP_LOGI(TAG, "Device sleep start, %ds\n", sleep_length);
    esp_sleep_enable_timer_wakeup(sleep_length * 1000000);
    esp_deep_sleep_start();
}
