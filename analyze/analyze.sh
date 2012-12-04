#!/bin/sh

set -e

NOW=$(date +%Y%m%d%H%M%S)

# paths & environment
scriptpath=$(dirname $0)
makepath=$(readlink -ne "${scriptpath}/../logger/")
makefile="${makepath}/Makefile"
objcopy=objcopy
make=make
dnld_file="logger.flash-read.hex"

# arg1 is the name of the output file
readchip()
{
    test -r "${makefile}"
    ${make} -f "${makefile}" flash-read
    ${objcopy} -I ihex -O binary "${dnld_file}" "${1}"
    rm "${dnld_file}"
}

extract()
{
    local datfile="${1%.bin}.csv"
    ${scriptpath}/parselog.pl --input "${1}" --output "${datfile}"
}

plot()
{
    local filtercmd=""
    local output=""
    case "${1}" in
	*.csv)
	    sep=','
	    output="${1%.csv}.png"
	    ;;
	*.dat)
	    sep='\t'
	    output="${1%.dat}.png"
	    ;;
	*)
	    echo "Format ${1} not supported" 2>&1
	    exit 1
	    ;;
    esac

    gnuplot -e "
set datafile separator \"${sep}\";
set term png;
set output \"${output}\";
plot \"${1}\" u 1:2 w lines;
"
}

readchip "${NOW}.bin"
extract  "${NOW}.bin"
plot     "${NOW}.csv"
