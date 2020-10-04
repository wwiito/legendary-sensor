/*
 * spifs.h
 *
 *  Created on: May 12, 2020
 *      Author: wit
 */

#ifndef MAIN_CODE_SPIFS_H_
#define MAIN_CODE_SPIFS_H_

#include <memory>

class spifs {
public:
	spifs(const std::string &pName, const std::string &mountPath, int maxFiles = 5, bool format_after_failure = false);
	const std::string & get_mount_path();
	~spifs();
private:
	void mount_partition();
	const std::string &partition_name;
	const std::string &mount_path;
	int max_files;
	bool format_after_failure;
	std::string TAG;

};

#endif /* MAIN_CODE_SPIFS_H_ */
