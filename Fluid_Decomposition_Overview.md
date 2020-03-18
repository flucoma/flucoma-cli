# fluid decomposition: play with sonic slices, layers and objects

The Fluid Decomposition toolbox provides an open-ended, loosely coupled set of objects to break up and analyse sound in terms of slices (segments in time), layers (superositions in time and frequency) and objects (configurable or discoverable patterns in sound).

## Slices
#### `fluid-ampslice` and `fluid-ampgate`
Slice by amplitude envelope

#### `fluid-onsetslice`  
Slice by onsets in the spectral domain

#### `fluid-noveltyslice`  
Slice by estimated novelty on a choice of features

#### `fluid-transientslice`  
Slice by transient modelling

## Layers
#### `fluid-sines`  
Decompose into sines + residual

#### `fluid-transients`  
Decompose into transients  + residual

#### `fluid-hpss`  
Decompose into 'harmonic' and 'percussive' layers

## Objects
#### `fluid-nmf`  
Use Nonnegative Matrix Factorisation to explore and decompose sounds

## Descriptors
#### `fluid-loudness`  
Report amplitude and peak, or r128 loudness and true peak

#### `fluid-pitch`  
A suite of pitch estimators

#### `fluid-melbands`  
Energy across Mel bands

#### `fluid-mfcc`  
Mel Frequency Ceptstral Coefficients

#### `fluid-spectralshape`  
Summary measures of spectral shape

#### `fluid-stats`  
Statistics of files


This toolbox was made possible thanks to the FluCoMa project ( http://www.flucoma.org/ ) funded by the European Research Council ( https://erc.europa.eu/ ) under the European Union’s Horizon 2020 research and innovation programme (grant agreement No 725899).
