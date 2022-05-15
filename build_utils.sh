#!/bin/bash

lastFolderName=$folderName
folderName=${PWD##*/}

source log.sh
last_log_prefix=$log_prefix
log_prefix="-- [${folderName} dependencies script]: "

buildFolderPrefix="Build"
onlyLibArg=" "
cmakeTestsArg=" "
cmakeGppArg=" "
buildConfig="Debug"
logArg=" -DLOG_ON=ON"
build=""
rootDirectory="."
onlyConfig=false

parse_args()
{
	argIndex=0
	for arg in "$@" 
	do
		#echo "arg[$argIndex]: '$arg'"
		
		if [[ $argIndex -eq 0 ]]; then
			rootDirectory=$arg
		else
			if [[ "$arg" == "only-lib" ]]; then
				log "'only-lib' option passed. Build only library without tests" " --"
				onlyLibArg=" only-lib"
				cmakeTestsArg=" "
			elif [[ "$arg" == "g++" ]]; then
				log "'g++' option passed. Build with g++ compiler" " --"
				cmakeGppArg= -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gpp
				gppArg="g++"
				buildFolderPrefix="Build-g++"
			elif [[ "$arg" == "no-log" ]]; then
				log "'no-log' option passed. Turn off LOG_ON compile definition" " --"
				logArg=" "
			elif [[ "$arg" == "release" ]]; then
				log "'release' option passed. Set Release build type" " --"
				buildConfig="Release"
			elif [[ "$arg" == "configure" ]]; then
				log "'configure' option passed. Will not build the project. Only make the config" " --"
				onlyConfig=true
			fi
		fi	
		argIndex=$((argIndex + 1))
	done
}

folderName=$lastFolderName
log_prefix=$last_log_prefix
