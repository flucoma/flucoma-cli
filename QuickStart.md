# Instructions for the Command-Line Interface version of the Fluid.buf* algorythms

## Note: if you never used the Terminal, this is not for you yet, as these instructions are very terse.

## How to start:

1) copy the content of /bin wherever

2) if you have not copied it in one of your /bin folders, add the path of where it is to your shell $PATH by typing:
```export PATH="XXX:$PATH"```
where XXX is the complete path to the folder that contains your Flucoma-CLI apps.

3) Use the CLI by entering the function and a fulwl list of arguments, preceded with the hyphen (-). Default values are in place, but you need at least a source and destination paths. These need to be full paths.

For instance:
```
hpss -source FULLPATHTOASOUND -harmonic ~/Desktop/harm.wav -percussive ~/Desktop/perc.wav
```

will put on the Desktop 2 files from the default HPSS settings, and:

```
nmf -source FULLPATHTOASOUND -rank 5 -resynth ~/Desktop/ranks.wav
```

will put on the desktop a single 5-channel audio file with the resynthesised 5 ranks requested.

#### Enjoy!


## Known limitations:
- only outputs WAV 32bit float files for now.
- do not support relative paths.
- no man page: references needs to be taken from your favourite CCE, but SuperCollider has the right spelling (in camelCase, not hard to guess from the Max helpfile)
