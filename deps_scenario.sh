#!/bin/bash

deps_scenario()
{
	local THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	local folderName=${THIS_DIR##*/}

	source $THIS_DIR/log.sh
	local log_prefix="-- [${folderName} deps_scenario script]: "

	source $THIS_DIR/dependencies.sh
	source $THIS_DIR/deps_config.sh

	log "deps_scenario of folder '${PWD##*/}' started" " -"
	download_dependency "DataModelBuilder" "$depsLocation" "git@github.com:skalexey/DataModelBuilder.git"
	local currentDir=${PWD}
	log "deps_scenario of folder '${PWD##*/}' finished" " -"
	source "$depsLocation/DataModelBuilder/Core/deps_scenario.sh"
	cd $currentDir
}

deps_scenario $@