# Fluid Decomposition: play with sonic slices, layers and objects

The Fluid Decomposition toolbox provides an open-ended, loosely coupled set of objects to break up and analyse sound in terms of slices (segments in time), layers (superpositions in time and frequency) and objects (configurable or discoverable patterns in sound).

## File Formats
The programs can read audio in Wav or AIFF format, and write either audio or CSV files (determined by the extension given to the relevant argument).

## Slices
#### ampslice
Slice by amplitude envelope

#### onsetslice
Slice by onsets in the spectral domain

#### noveltyslice
Slice by estimated novelty on a choice of features

#### transientslice
Slice by transient modelling

## Layers
#### sines
Decompose into sines + residual

#### transients
Decompose into transients  + residual

#### hpss
Decompose into 'harmonic' and 'percussive' layers

## Objects
#### nmf
Use Nonnegative Matrix Factorisation to explore and decompose sounds

## Descriptors
#### loudness
Report amplitude and peak, or r128 loudness and true peak

#### pitch
A suite of pitch estimators

#### melbands
Energy across Mel bands

#### mfcc
Mel Frequency Ceptstral Coefficients

#### spectralshape
Summary measures of spectral shape

#### stats
Statistics of files


This toolbox was made possible thanks to the FluCoMa project ( http://www.flucoma.org/ ) funded by the European Research Council ( https://erc.europa.eu/ ) under the European Union’s Horizon 2020 research and innovation programme (grant agreement No 725899).
