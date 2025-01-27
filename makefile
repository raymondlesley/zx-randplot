# makefile - randplot

# builds a TZX format output file
#
# * "compiles" loader from Sinclair BASIC
# * cross-compiles C program into TAP (includes a loader I don't want)
# * splits cross-compiled TAP
# * builds TZX from component parts

cc = snap run z88dk.zcc
cc_opts = +zx -clib=sdcc_iy --opt-code-size -create-app

bas2tap = ../bas2tap/bas2tap
bas2tap_opts = -a10

tzxmerge = ../../Python/tzxtools/tzxmerge.py
tzxsplit = ../../Python/tzxtools/tzxsplit.py
split_opts = -1

all: randplot.tzx clean

randplot.tzx: loader.tap randplot.tap
	rm -f *.tzx
	${tzxsplit} ${split_opts} randplot.tap
	${tzxmerge} loader.tap randplot-001.tzx -o randplot.tzx

randplot.tap: randplot.c
	${cc} ${cc_opts} -o $(basename $@) -startup=0 $<

loader.tap: loader.bas
	${bas2tap} ${bas2tap_opts} -s"$(basename $@)" $< $(basename $@).tap

clean:
	rm -f *.bin randplot Loader-001.tzx randplot-001.tzx

unmake: clean
	rm randplot.tzx randplot.tap loader.tap
