rm -rf stripestest && mkdir stripestest && ./ilbm2raw ../imgtest/stripes.iff ./stripestest/stripes.raw && diff ./stripestest/stripes.raw ../imgout/stripes.raw && rm -rf ./stripestest
