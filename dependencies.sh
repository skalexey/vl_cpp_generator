#!/bin/bash

folderName=${PWD##*/}

source log.sh
log_prefix="-- [${folderName} dependencies script]: "

download_dependency()
{
	dep_dir_name=$1
	deps_path=${2}
	deps_path="${deps_path//\~/$HOME}"
	repo=$3

	log "Resolve dependency directory '${dep_dir_name}'" " --"

	if [[ ! -d "${deps_path}" ]]; then
		log "Dependencies directory '${deps_path}' does not exist. Try to create it..." " ---"
		mkdir -p "${deps_path}"
		retval=$?
		if [ $retval -ne 0 ]; then
			log "Directory '${deps_path}' creation error" " ---"
			exit 1
		else
			log "Created" " ---"
		fi
	fi

	if [[ ! -d "$deps_path/$dep_dir_name" ]]; then
		log "Dependency directory '$dep_dir_name' does not exist. Download..." " ---"
		cur_path=$(PWD)
		cd "$deps_path"
		git clone ${repo}
		cd "${cur_path}"
		retval=$?
		if [ $retval -ne 0 ]; then
			log "Directory '${dep_name}' creation error" " ---"
			exit 1
		else
			log "Completed download of dependency '$dep_dir_name'" " ---"
		fi
	else
		log "Dependency '$dep_dir_name' is already downloaded" " ---"
	fi
}
