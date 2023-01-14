#!/usr/bin/env python3
## Copyright (c) 2022 Daniel Tabor
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
## 1. Redistributions of source code must retain the above copyright
##	notice, this list of conditions and the following disclaimer.
## 2. Redistributions in binary form must reproduce the above copyright
##	notice, this list of conditions and the following disclaimer in the
##	documentation and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
## ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
## FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
## DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
## OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
## HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
## LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
## OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
## SUCH DAMAGE.
##
import sys
import os

def usage():
	filename = os.path.basename(sys.argv[0])
	print("Usage:")
	print("%s [-h] source_directory" % filename)
	print("")
	sys.exit(1)

def main():
	if "-h" in sys.argv or len(sys.argv) != 2 or not os.path.isdir(sys.argv[1]):
		usage()
	src_dir = sys.argv[1]
	resource_filenames = os.listdir(src_dir)
	resource_filenames.sort()
	c_fp = open("resources.c","w")
	h_fp = open("resources.h","w")
	h_fp.write('#include "res.h"\n')
	h_fp.write('#define RESOURCE_COUNT %d\n' % len(resource_filenames))
	c_fp.write("#define __RESOURCES_C__\n")
	c_fp.write('#include "res.h"\n')
	count = 0
	lengths = []
	for filename in resource_filenames:
		path = os.path.join(src_dir,filename)
		f = open(path,"rb")
		data = f.read()
		defname = filename.upper().replace(".","_")
		h_fp.write("#define %s %d\n" % (defname,count))
		c_fp.write("static uint8_t resource%d[] = {" % count)
		for i in range(len(data)):
			c_fp.write("0x%02X" % data[i])
			if i != len(data)-1:
				c_fp.write(",")
		c_fp.write("};\n")
		lengths.append(len(data))
		count += 1
	h_fp.write("#ifndef __RESOURCES_C__\n")
	h_fp.write("extern resource_t resources[];\n")
	h_fp.write("#endif\n")
	c_fp.write("resource_t resources[] = {\n")
	for i in range(len(lengths)):
		c_fp.write("(resource_t){.len=%d,.data=resource%d}" % (lengths[i],i))
		if i != len(lengths)-1:
			c_fp.write(",\n")
		else:
			c_fp.write("\n")
	c_fp.write("};\n")

if __name__ == "__main__":
	main()
