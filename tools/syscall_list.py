import requests
import sys

# read syscall table from https://github.com/torvalds/linux/blob/29dcea88779c856c7dc92040a0c01233263101d4/arch/x86/entry/syscalls/syscall_64.tbl
url = 'https://raw.githubusercontent.com/torvalds/linux/29dcea88779c856c7dc92040a0c01233263101d4/arch/x86/entry/syscalls/syscall_64.tbl'

# get the file
r = requests.get(url)

# split into lines
lines = r.text.split('\n')

pattern = """#pragma once

enum class Syscall{{
{}}};

inline std::string syscallToString(Syscall sc){{
    switch(sc){{
{}            default: return "UNKNOWN";
    }}
}}
"""

outfile_name = "Syscall.hpp"
if len(sys.argv) > 1:
    outfile_name = sys.argv[1]
else:
    print(f"Generating syscall list")

syscall_list = ""
syscall_string_list = ""

num_syscalls = 0
for line in lines:
    if line.startswith('#'): continue

    entry_point = ""
    if (len(line.split()) == 3):
        number, type, name = line.split()
    elif (len(line.split()) == 4):
        number, type, name, entry_point = line.split()
    else: continue

    if (type == "common" or type == "64"):
        syscall_list += f"     {name} = {number},\n"
        syscall_string_list += f"            case Syscall::{name}: return \"{name}\";\n"
        num_syscalls += 1

print(f"Wrote {num_syscalls} syscalls to \"{outfile_name}\".")

outfile = open(outfile_name, "w")
outfile.write(pattern.format(syscall_list, syscall_string_list))
outfile.close()