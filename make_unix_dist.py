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

import os
import sys
import subprocess
import shutil

def main():
	exe_path = sys.argv[1]
	dst_path = sys.argv[2]
	if not os.path.exists(dst_path):
		os.makedirs(dst_path)

	print("[dist added] %s" % exe_path)
	shutil.copy(exe_path,dst_path)

	proc = subprocess.Popen(["ldd",exe_path],stdout=subprocess.PIPE)
	proc.wait()
	while True:
		line = proc.stdout.readline().decode()
		if not len(line):
			break
		if "=>" in line:
			line = line.strip().split(" ")
			src_path = line[2]
			print("[dist added] %s" % src_path)
			shutil.copy(src_path,dst_path)

if __name__ == "__main__":
	main()
