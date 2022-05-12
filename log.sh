#!/usr/bin/bash

log_prefix=""
log_postfix=""
log()
{
	[ -z "$1" ] && exit 0
	[ ! -z "$2" ] && local_prefix=$2$log_prefix || local_prefix=$log_prefix
	[ ! -z "$3" ] && local_postfix=$log_postfix$3 || local_postfix=$log_postfix
	echo "${local_prefix}$1${local_postfix}"
}