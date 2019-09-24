# ilbm2raw

ilbm2raw is a simple tool to convert Amiga IFF image files to raw data needed for Amiga programming.  

ilbm2raw is based on code from Sebastiano Vigna from Italian Amiga Magazine issue 11 disk, his code has been enanched to support Byterun1 compression, non-interleaved output images and palette recording.

### installation

In order to compile and install ilbm2raw you will need a regular gcc, the whole program has almost no dependencies and it should work on any modern Linux distro, just type the canonical
```
  ./configure
  make
  sudo make install
```

and you are done.

### usage

Typing ilbm2raw -h will display a little help screen that is quite self-explanatory

```
  Usage: ilbm2raw inputIFFFile outputRAWFile [OPTIONS]
  OPTIONS:
    -a		     : Resulting file will be created according to ACE (Amiga C Engine) specifications (https://github.com/AmigaPorts/ACE)
    -p outfile       : Write resulting palette into outfile
    -i 		     : Output in interleaved mode (raw mode default)
    -v		     : Be verbose

```

