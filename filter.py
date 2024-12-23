#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

selected_sources = [ int(arg) for arg in sys.argv[1:] ]

mappings_filename = 'libsig-map.out'
mappings = []
with open(mappings_filename, 'r') as mappings_file:
    for i, line in enumerate(mappings_file, 1):
        if i in selected_sources:
            source, address, size = line.strip().split(':')
            start = int(address, 16)
            size = int(size)
            end = start + size
            mappings.append((source, start, end))


functions_filename = 'libsig-functions.out'
filtered_functions = []
with open(functions_filename, 'r') as functions_file:
    for line in functions_file:
        addr, name = line.strip().split(':')
        addr = int(addr, 16)

        if name in filtered_functions:
            continue

        for source, start, end in mappings:
            if start <= addr < end:
                filtered_functions.append(name)
                break

print(filtered_functions, sep=' ')
