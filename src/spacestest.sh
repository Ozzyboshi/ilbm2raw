rm -rf spacestest && mkdir spacestest && ./ilbm2raw ../imgtest/Sfondo\ Titolo\ Chiaro.iff ./spacestest/Sfondo\ Titolo\ Chiaro.raw && diff ./spacestest/Sfondo\ Titolo\ Chiaro.raw ../imgout/Sfondo\ Titolo\ Chiaro.raw && rm -rf ./spacestest
