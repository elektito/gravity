from sys import argv
import os

output_file = argv[1]
prefix = argv[2]
resources_path = prefix + argv[3]

with open(output_file, 'w') as out:
    out.write('#!/bin/sh\n{}/bin/gravity-bin {}\n'.format(prefix, resources_path))
