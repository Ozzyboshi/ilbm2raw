bin_PROGRAMS = ilbm2raw
ilbm2raw_SOURCES = ilbm2raw.c

# Unit testing
check_SCRIPTS = greptest.sh
TESTS = $(check_SCRIPTS)

greptest.sh:
	echo './ilbm2raw --help | grep "Usage"' > greptest.sh
	chmod +x greptest.sh

CLEANFILES = greptest.sh
