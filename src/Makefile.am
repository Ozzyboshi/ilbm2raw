bin_PROGRAMS = ilbm2raw
ilbm2raw_SOURCES = ilbm2raw.c

AM_CFLAGS = -Wno-unused-result

# Unit testing
check_SCRIPTS = greptest.sh acenoninterleavedtest.sh rawnoninterleavedtest1.sh rawnoninterleavedtest2.sh rawinterleavedonelineswaptest.sh stripestest.sh spacestest.sh bpltest.sh
TESTS = $(check_SCRIPTS)

greptest.sh:
	echo './ilbm2raw --help | grep "Usage"' > greptest.sh
	chmod +x greptest.sh

acenoninterleavedtest.sh:
	echo 'mkdir acenoninterleavedtestout && ./ilbm2raw ../imgtest/rectintro.iff ./acenoninterleavedtestout/rectintro.bm -a -p ./acenoninterleavedtestout/rectintro.plt && diff ./acenoninterleavedtestout/rectintro.bm ../imgout/rectintro.bm && diff ./acenoninterleavedtestout/rectintro.plt ../imgout/rectintro.plt && rm -rf ./acenoninterleavedtestout' > acenoninterleavedtest.sh
	chmod +x acenoninterleavedtest.sh

rawnoninterleavedtest1.sh:
	echo 'mkdir rawnoninterleavedtest1 && ./ilbm2raw ../imgtest/flowers320x256x8.iff ./rawnoninterleavedtest1/flowers320x256x8.raw -p ./rawnoninterleavedtest1/flowers320x256x8.plt && diff ./rawnoninterleavedtest1/flowers320x256x8.raw ../imgout/flowers320x256x8.raw && diff ./rawnoninterleavedtest1/flowers320x256x8.plt ../imgout/flowers320x256x8.plt && rm -rf ./rawnoninterleavedtest1' > rawnoninterleavedtest1.sh
	chmod +x rawnoninterleavedtest1.sh

rawnoninterleavedtest2.sh:
	echo 'mkdir rawnoninterleavedtest2 && ./ilbm2raw ../imgtest/valchiria320x224x16.iff ./rawnoninterleavedtest2/valchiria320x224x16.raw -p ./rawnoninterleavedtest2/valchiria320x224x16.plt && diff ./rawnoninterleavedtest2/valchiria320x224x16.raw ../imgout/valchiria320x224x16.raw && diff ./rawnoninterleavedtest2/valchiria320x224x16.plt ../imgout/valchiria320x224x16.plt && rm -rf ./rawnoninterleavedtest2' > rawnoninterleavedtest2.sh
	chmod +x rawnoninterleavedtest2.sh

rawinterleavedonelineswaptest.sh:
	echo 'rm -rf rawinterleavedonelineswaptest && mkdir rawinterleavedonelineswaptest && ./ilbm2raw -s 0,1 ../imgtest/simpleinterleaved.iff ./rawinterleavedonelineswaptest/simpleinterleaved.raw -p ./rawinterleavedonelineswaptest/simpleinterleaved.plt && diff ./rawinterleavedonelineswaptest/simpleinterleaved.raw ../imgout/simpleinterleaved.raw && diff ./rawinterleavedonelineswaptest/simpleinterleaved.plt ../imgout/simpleinterleaved.plt && rm -rf ./rawinterleavedonelineswaptest' > rawinterleavedonelineswaptest.sh
	chmod +x rawinterleavedonelineswaptest.sh

stripestest.sh:
	echo 'rm -rf stripestest && mkdir stripestest && ./ilbm2raw ../imgtest/stripes.iff ./stripestest/stripes.raw && diff ./stripestest/stripes.raw ../imgout/stripes.raw && rm -rf ./stripestest' > stripestest.sh
	chmod +x stripestest.sh

spacestest.sh:
	echo 'rm -rf spacestest && mkdir spacestest && ./ilbm2raw ../imgtest/Sfondo\ Titolo\ Chiaro.iff ./spacestest/Sfondo\ Titolo\ Chiaro.raw && diff ./spacestest/Sfondo\ Titolo\ Chiaro.raw ../imgout/Sfondo\ Titolo\ Chiaro.raw && rm -rf ./spacestest' > spacestest.sh
	chmod +x spacestest.sh

maskstest.sh:
	echo 'rm -rf maskstest && mkdir maskstest && ./ilbm2raw ../imgtest/e.iff ./maskstest/e.raw -m ./maskstest/e.mask -r ./maskstest/e.maskreverse && diff ./maskstest/e.raw ../imgout/e.raw && diff ./maskstest/e.mask ../imgout/e.mask && diff ./maskstest/e.maskreverse ../imgout/e.maskreverse && rm -rf ./maskstest' > maskstest.sh
	chmod +x maskstest.sh

bpltest.sh:
	echo 'rm -rf bpltest && mkdir bpltest && ./ilbm2raw -f -b ../imgtest/rcfloppy.iff ./bpltest/rcfloppy.raw && diff ./bpltest/rcfloppy.raw ../imgout/rcfloppy.raw && diff ./bpltest/rcfloppy.raw.0 ../imgout/rcfloppy.raw.0 && diff ./bpltest/rcfloppy.raw.1 ../imgout/rcfloppy.raw.1 && diff ./bpltest/rcfloppy.raw.2 ../imgout/rcfloppy.raw.2 && rm -rf ./bpltest' > bpltest.sh
	chmod +x bpltest.sh

CLEANFILES = greptest.sh acenoninterleavedtest.sh rawnoninterleavedtest1.sh rawnoninterleavedtest2.sh rawinterleavedonelineswaptest.sh stripestest.sh spacestest.sh maskstest.sh spacestest.sh bpltest.sh
