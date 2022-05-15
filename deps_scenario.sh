#!/bin/bash

deps_scenario()
{
	local folderName=${PWD##*/}

	source log.sh
	local log_prefix="-- [${folderName} deps_scenario script]: "

	source dependencies.sh
	source deps_config.sh

	log "deps_scenario of folder '${PWD##*/}' started" " -"
	download_dependency "DataModelBuilder" "$depsLocation" "git@github.com:skalexey/DataModelBuilder.git"
	local currentDir=${PWD}
	log "deps_scenario of folder '${PWD##*/}' finished" " -"
	cd $currentDir
}

deps_scenario $@