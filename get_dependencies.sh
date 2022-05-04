#!/bin/sh

echo " --- Check for dependencies"

if [ ! -f "deps_config.sh" ]; then
	echo " ---- No dependencies"
	exit
fi
source deps_config.sh

enterDirectory=${pwd}

./deps_scenario.sh

cd "${enterDirectory}"