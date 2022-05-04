#!/bin/sh

download_dependency()
{
	dep_dir_name=$1
	deps_path=${2}
	deps_path="${deps_path//\~/$HOME}"
	repo=$3

	echo " ---- Resolve dependency directory '${dep_dir_name}'"  

	if [[ ! -d "${deps_path}" ]]; then
		echo " ----- Dependencies directory '${deps_path}' does not exist. Try to create it..."
		mkdir -p "${deps_path}"
		retval=$?
		if [ $retval -ne 0 ]; then
			echo " ----- Directory '${deps_path}' creation error"
			exit 1
		else
			echo " ----- Created"
		fi
	fi

	if [[ ! -d "$deps_path/$dep_dir_name" ]]; then
		echo " ----- Dependency directory '$dep_dir_name' does not exist. Download..."
		cur_path=$(PWD)
		cd "$deps_path"
		git clone ${repo}
		cd "${cur_path}"
		retval=$?
		if [ $retval -ne 0 ]; then
			echo " ----- Directory '${dep_name}' creation error"
			exit 1
		else
			echo " ----- Completed download of dependency '$dep_dir_name'"
		fi
	else
		echo " ----- Dependency '$dep_dir_name' is already downloaded"
	fi
}
