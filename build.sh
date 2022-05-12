#!/bin/bash

source log.sh
folderName=${PWD##*/}
log_prefix="-- [${folderName} build script]: "

log "Build for OS: $OSTYPE" " -" " ---"

extraArg=" "
extraArgWin=$extraArg
extraArgMac=$extraArg

if [ -f "deps_config.sh" ]; then
	source deps_config.sh
fi

source build_config.sh

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
	generatorArg=" "
elif [[ "$OSTYPE" == "darwin"* ]]; then
	# Mac OSX
	generatorArg=" -GXcode"
	extraArg=$extraArgMac
elif [[ "$OSTYPE" == "cygwin" ]]; then
	generatorArg=" "
	extraArg=$extraArgWin
elif [[ "$OSTYPE" == "msys" ]]; then
	generatorArg=" "
	extraArg=$extraArgWin
elif [[ "$OSTYPE" == "win32" ]]; then
	generatorArg=" "
	extraArg=$extraArgWin
elif [[ "$OSTYPE" == "freebsd"* ]]; then
	generatorArg=" "
else
	generatorArg=" "
fi

[ ! -z "$extraArg" ] && log "Extra arguments: '$extraArg'" " -"

source build_utils.sh

parse_args $@

enterDirectory=${pwd}

if [ -f "get_dependencies.sh" ]; then
	source get_dependencies.sh $@
	retval=$?
	if [ $retval -ne 0 ]; then
		log "Dependencies resolution error" " --"
		exit 1
	else
		log "Done with dependencies" " --"
		cd "$enterDirectory"
	fi
fi

[ ! -d "$rootDirectory" ] && log "Non-existent project directory passed '$rootDirectory'" " -" && exit 1 || cd "$rootDirectory"

if [[ "$rootDirectory" != "." ]]; then
	folderName=$rootDirectory
fi

echo "--- [${folderName}]: Configure with CMake"

build="${buildFolderPrefix}-cmake"

log "Output directory: '$build'" " -"

[ ! -d "$build" ] && mkdir $build || echo "	already exists"
cd $build

cmake ..$generatorArg$logArg$extraArg

retval=$?
if [ $retval -ne 0 ]; then
	log "CMake configure error" " -"
	cd "$enterDirectory"
	exit
else
	log "CMake configuring has been successfully done" " -"
fi

[ "$onlyConfig" == true ] && log "Exit without build" " -" && exit || log "Run cmake --build" " -"

cmake --build . --config=$buildConfig

retval=$?
if [ $retval -ne 0 ]; then
	log "CMake build error" " -"
	cd "$enterDirectory"
	exit
else
	log "CMake building is successfully done" "-" " ---"
fi

cd "$enterDirectory"

log "Finished build" " -" " ---"