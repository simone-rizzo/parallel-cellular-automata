#!/bin/bash
for((i=0; i<100; i++)); do ./cellular_automata_ff; done | awk '{$sum = $sum + $5} END {print $sum/NR}'