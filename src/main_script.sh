#!/bin/bash
for((i=0; i<100; i++)); do ./main; done | awk '{$sum = $sum + $5} END {print $sum}'