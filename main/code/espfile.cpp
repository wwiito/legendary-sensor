/*
 * espfile.cpp
 *
 *  Created on: May 14, 2020
 *      Author: wit
 */
#include "esp_log.h"

#include "espfile.h"
#include "spifs.h"

#include <string.h>
#include <string>

esp_file::esp_file(spifs &fs, const std::string fname):
f(NULL),
fbuf(NULL)
{
	this->full_file_name = fs.get_mount_path() + fname;

	ESP_LOGI(this->full_file_name.data(), "%s", this->full_file_name.data());

	if (stat(this->full_file_name.data(), &this->s)) {
		ESP_LOGE("FAIL", "Failed to stat file!");
		throw std::runtime_error("Failed to stat file");
	}

	ESP_LOGI(this->full_file_name.data(), "File length: %d", (int)this->s.st_size);
}

esp_file::~esp_file() {
	// TODO Auto-generated destructor stub
}

void esp_file::open() {
	FILE *f = fopen(this->full_file_name.data(), "r");
	if (f == NULL) {
		ESP_LOGE(this->full_file_name.data(), "Failed to open file");
		throw std::runtime_error("Failed to open file");
	}
	this->f = f;
}
void esp_file::close() {
	if (this->f != NULL)
		fclose(this->f);
}
uint8_t* esp_file::get_file_buffer() {
	if (this->f == NULL)
		open();

	fbuf = new uint8_t[s.st_size + 1];
	if (fbuf == NULL)
		throw std::runtime_error("Memory allocation failed");
	memset(fbuf, 0x00, s.st_size+1);

	int len = fread(fbuf, s.st_size, 1, f);
	ESP_LOGI(full_file_name.data(), "Read: %d bytes",len );

	return fbuf;
}
void esp_file::free_file_buffer() {
	if (this->fbuf == NULL)
		return;

	delete [] fbuf;
	close();
}


void esp_file::debug_display_file_content() {
	open();
	uint8_t * buf = get_file_buffer();
	ESP_LOGI(full_file_name.data(), "%s",fbuf );
	free_file_buffer();
	close();
}
