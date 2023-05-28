#!/bin/bash

mkdir -p "$2"
cd $2 || exit 1
rm -rf train user *.out
i=0
while (true); do
	((++i))
	input_file=${1}/${2}/${i}.in
	ans_file=${1}/${2}/${i}.out
	output_file=${i}.out
	if [ ! -f "${input_file}" ]; then
		break
	fi
	if ! $3 <"${input_file}" >"${output_file}"; then
		echo -e "\033[31mCase ${i}: Program do not return 0\033[0m"
		exit 2
	fi
	if ! diff -ZB "${output_file}" "${ans_file}" >/dev/null; then
		echo -e "\033[31mCase ${i}: Output is not correct\033[0m"
		exit 1
	fi
	echo "${i} ok"
done