rm -rf rawinterleavedonelineswaptest && mkdir rawinterleavedonelineswaptest && ./ilbm2raw -s 0,1 ../imgtest/simpleinterleaved.iff ./rawinterleavedonelineswaptest/simpleinterleaved.raw -p ./rawinterleavedonelineswaptest/simpleinterleaved.plt && diff ./rawinterleavedonelineswaptest/simpleinterleaved.raw ../imgout/simpleinterleaved.raw && diff ./rawinterleavedonelineswaptest/simpleinterleaved.plt ../imgout/simpleinterleaved.plt && rm -rf ./rawinterleavedonelineswaptest
