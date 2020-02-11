bin_PROGRAMS = ilbm2raw
ilbm2raw_SOURCES = ilbm2raw.c

# Unit testing
check_SCRIPTS = greptest.sh acenoninterleavedtest.sh rawnoninterleavedtest1.sh rawnoninterleavedtest2.sh
TESTS = $(check_SCRIPTS)

greptest.sh:
	echo './ilbm2raw --help | grep "Usage"' > greptest.sh
	chmod +x greptest.sh

acenoninterleavedtest.sh:
	mkdir acenoninterleavedtestout
	echo './ilbm2raw ../imgtest/rectintro.iff ./acenoninterleavedtestout/rectintro.bm -a -p ./acenoninterleavedtestout/rectintro.plt && diff ./acenoninterleavedtestout/rectintro.bm ../imgout/rectintro.bm && diff ./acenoninterleavedtestout/rectintro.plt ../imgout/rectintro.plt && rm -rf ./acenoninterleavedtestout' > acenoninterleavedtest.sh
	chmod +x acenoninterleavedtest.sh
	
rawnoninterleavedtest1.sh:
	mkdir rawnoninterleavedtest1
	echo './ilbm2raw ../imgtest/flowers320x256x8.iff ./rawnoninterleavedtest1/flowers320x256x8.raw -p ./rawnoninterleavedtest1/flowers320x256x8.plt && diff ./rawnoninterleavedtest1/flowers320x256x8.raw ../imgout/flowers320x256x8.raw && diff ./rawnoninterleavedtest1/flowers320x256x8.plt ../imgout/flowers320x256x8.plt && rm -rf ./rawnoninterleavedtest1' > rawnoninterleavedtest1.sh
	chmod +x rawnoninterleavedtest1.sh
	
rawnoninterleavedtest2.sh:
	mkdir rawnoninterleavedtest2
	echo './ilbm2raw ../imgtest/valchiria320x224x16.iff ./rawnoninterleavedtest2/valchiria320x224x16.raw -p ./rawnoninterleavedtest2/valchiria320x224x16.plt && diff ./rawnoninterleavedtest2/valchiria320x224x16.raw ../imgout/valchiria320x224x16.raw && diff ./rawnoninterleavedtest2/valchiria320x224x16.plt ../imgout/valchiria320x224x16.plt && rm -rf ./rawnoninterleavedtest2' > rawnoninterleavedtest2.sh
	chmod +x rawnoninterleavedtest2.sh

CLEANFILES = greptest.sh acenoninterleavedtest.sh rawnoninterleavedtest1.sh rawnoninterleavedtest2.sh
