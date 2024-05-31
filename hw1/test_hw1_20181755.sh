#!/usr/bin/env bash

ga="getaddrinfo"

gcc hw1_20181755.c -o $ga


file="sites.txt"

while IFS= read -r line; do
    echo -e "\n$line"
    ./$ga $line
done < "$file"