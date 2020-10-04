/*
 * spifs.cpp
 *
 *  Created on: May 12, 2020
 *      Author: wit
 */

#include "spifs.h"
#include "string.h"

#include "esp_log.h"
#include "esp_spiffs.h"

#include "dirent.h"


spifs::spifs(const std::string &pName, const std::string &mountPath, int max_files, bool format_after_failure):
partition_name(pName),
mount_path(mountPath)
{
	this->max_files = max_files;
	this->format_after_failure = format_after_failure;
	this->TAG = std::string("spifs_") + std::string(pName);

	this->mount_partition();

	ESP_LOGI(this->TAG.data(), "Initialized SPIFSS");

	DIR *d = opendir(mountPath.data());
	struct dirent *ent;
	while ((ent = readdir (d)) != NULL) {
		ESP_LOGI(TAG.data(), "fname: %s", ent->d_name);
	}

}

void spifs::mount_partition() {
    esp_vfs_spiffs_conf_t conf = {
      .base_path = this->mount_path.data(),
      .partition_label = this->partition_name.data(),
      .max_files = static_cast<size_t>(this->max_files),
      .format_if_mount_failed = this->format_after_failure,
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(this->TAG.data(), "Failed to mount or format filesystem\n");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(this->TAG.data(), "Failed to find SPIFFS partition\n");
        } else {
            ESP_LOGE(this->TAG.data(), "Failed to initialize SPIFFS (%s)\n", esp_err_to_name(ret));
        }
        throw std::runtime_error("Exception during mounting partition");
    }

}

const std::string & spifs::get_mount_path() {
	return this->mount_path;
}

spifs::~spifs() {
	// TODO Auto-generated destructor stub
}

