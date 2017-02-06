#!/bin/bash

grep -E "^record|^ .field\((DESC|INP|OUT)" $1 | awk ' BEGIN { RS="record\\("; FS=") {|)\n|, |) {\n  field\\((DESC|INP|OUT), "; OFS="\t" } { print $1,$2,$3,$4,$5 }' | grep -v "^$"

