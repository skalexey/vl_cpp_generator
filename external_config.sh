
log_info "vl_cpp_generator external_config.sh included"

function job()
{
	local THIS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
	source $THIS_DIR/os.sh

	if is_windows; then
		export vl_cpp_generator_deps="${HOME}/Projects"
	else
		export vl_cpp_generator_deps="${HOME}/Projects"
	fi
}

job $@