# Instructions for the Command-Line Interface version of the Fluid.buf* algorithms

## Note: if you never used the Terminal, this is not for you yet, as these instructions are very terse.

## How to start:

1) copy the content of /bin wherever you fancy

2) if you have not copied it in one of your /bin folders, add the path of where it is to your shell $PATH by typing:
```
export PATH="XXX:$PATH"
```
where XXX is the complete path to the folder that contains your FluCoMa-CLI apps.

3) Use the CLI by entering the function and a full list of arguments, preceded with the hyphen (-). Default values are in place, but you need at least a source and destination paths. These need to be full paths.

For instance:
```
hpss -source FULLPATHTOASOUND -harmonic ~/Desktop/harm.wav -percussive ~/Desktop/perc.wav
```

will put on the Desktop 2 files from the default HPSS settings, and:

```
nmf -source FULLPATHTOASOUND -rank 5 -resynth ~/Desktop/ranks.wav
```

will put on the desktop a single 5-channel audio file with the resynthesised 5 ranks requested.

4) an overview of the provided functions is available in the Fluid_Decomposition_Overview.md file, and an extensive documentation is available as a webpage in the /docs folder - add fluid.buf to a function.

#### Enjoy!


## Known limitations:
- only outputs WAV 32bit float files for now.
- do not support relative paths.
- no man page yet: references needs to be taken from the provided documentation in /docs

> This project has received funding from the European Research Council (ERC) under the European Union’s Horizon 2020 research and innovation programme (grant agreement No 725899).
