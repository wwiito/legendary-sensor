/*
 * espfile.h
 *
 *  Created on: May 14, 2020
 *      Author: wit
 */

#ifndef MAIN_CODE_ESPFILE_H_
#define MAIN_CODE_ESPFILE_H_

#include <stdio.h>
#include <sys/stat.h>

#include "spifs.h"

class esp_file {
public:
	esp_file(spifs &fs, const std::string fname);
	virtual ~esp_file();

	void debug_display_file_content();
	uint8_t* get_file_buffer();
	void free_file_buffer();

private:
	FILE *f;
	std::string full_file_name;
	struct stat s;

	uint8_t* fbuf;

	void open();
	void close();

};

#endif /* MAIN_CODE_ESPFILE_H_ */
