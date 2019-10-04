bin_PROGRAMS = ilbm2raw
ilbm2raw_SOURCES = ilbm2raw.c

# Unit testing
check_SCRIPTS = greptest.sh acenoninterleavedtest.sh
TESTS = $(check_SCRIPTS)

greptest.sh:
	echo './ilbm2raw --help | grep "Usage"' > greptest.sh
	chmod +x greptest.sh

acenoninterleavedtest.sh:
	mkdir acenoninterleavedtestout
	echo './ilbm2raw ../imgtest/rectintro.iff ./acenoninterleavedtestout/rectintro.bm -a -p ./acenoninterleavedtestout/rectintro.plt && diff ./acenoninterleavedtestout/rectintro.bm ../imgout/rectintro.bm && diff ./acenoninterleavedtestout/rectintro.plt ../imgout/rectintro.plt && rm -rf ./acenoninterleavedtestout' > acenoninterleavedtest.sh
	chmod +x acenoninterleavedtest.sh

CLEANFILES = greptest.sh acenoninterleavedtest.sh 
