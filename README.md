# Wave
C++ class for wave file creating, playing, plotting, modulating, and demodulating. 

### Disclaimer
This is not production-worthy code! View this simply as a proof-of-concept.

### Initialization
```
Wave(ResourceInterchangeFileFormatHeader fileHeader, WaveFormatHeader formatHeader, WaveDataHeader dataHeader, char * waveform);
Wave(ResourceInterchangeFileFormatHeader fileHeader, WaveFormatHeader formatHeader, WaveDataHeader dataHeader);
Wave(int sampleRate, double duration, char * waveform);
Wave(int sampleRate, double duration);
Wave(char * filename);
```
The `Wave` class can be initialized through the five different constructors:

1. Providing `Wave` file header structures with the supplied sampled waveform.
2. Providing `Wave` file header structures.
3. Specifying the `sampleRate` and `duration` of the supplied sampled waveform.
4. Specifying the `sampleRate` and `duration`.
5. Specifying the `Wave` file location.

Additional required file information, such as sampled waveform allocation and miscellaneous `Wave` file header fields, will be generated by the constructors upon execution.

### Playing
```
void Play();
```
The `Play` method allows Windows users to play the sampled waveform through the Win32 `PlaySound` function. The method requires all `Wave` file information to be filled out before the `PlaySound` function will work. All the necessary information should be provided through one of the constructors.


### Plotting
```
void Plot(double duration, int quality);
```
If the user has GNUPlot installed on their system and configured it in their `PATH`, this method will generate a graph of the current sampled waveform. The method requires a specified  `duration`, to prevent the entire waveform from being plotted, and `quality`, to determine the definition of the sampled waveform plot. The definition is determined by skipping every nth sample where n is specified through `quality`.

### Opening
```
void Open(char * filename);
```
If the user chooses to pick a new `Wave` file to load, the `Open` method can be called to load a new `Wave` file. All the neccesary information about the `Wave` file will be generated upon execution.

### Saving
```
void Save(char * filename);
```
The user can call the `Save` method to create a playable `Wave` file. This file can be played by any major audio software.

### Generating 
```
void Generate(double frequency, int type);
```
The `Generate` method allows the user to generate a sine or cosine wave of any `frequency` at the current `sampleRate` and `duration`. The type of waveform is specified through `type` using one of two constants: `SINE_WAVE` and `COSINE_WAVE`. The `SQUARE_WAVE` is currently not supported.

	### Modulating
```
void Modulate(double carrierFrequency, double modulationFrequency, double modulationIndex, int type);
void Modulate(double carrierFrequency, char * modulationWaveform, double modulationIndex, int type);
```
By calling the `Modulate` method, a `modulationFrequency` (the frequency to be modulated) can be amplitude or frequency modulated over a `carrierFrequency` using a certain `modulationIndex`. The type of modulation is specified through `type` using one of two constants: `FREQUENCY_MODULATION` and `AMPLITUDE_MODULATION`. The `modulationFrequency` can be substituted with a `modulationWaveform` which allows more complex sampled waveforms to be modulated rather than simple frequencies.

### Demodulating
```
void Demodulate(double carrierFrequency, char * carrierWaveform, double modulationIndex, int type);
```
The `Demodulate` method takes an already amplitude or frequency modulated sampled waveform and demodulates using a certain `carrierFrequency` and `modulationIndex`. Frequency demodulation is currently very sporatic so it should be used with caution.

### Setters/Getters
```
void SetSampleRate(int sampleRate);
int GetSampleRate();
void SetDuration(double duration);
double GetDuration();
void SetWaveform(char * waveform);
char * GetWaveform();
```
These are pretty self-explanatory methods that allow the user to set and get important parts of the `Wave` file.

### Deinitialization
```
~Wave();
```
The allocated sampled waveform will be deallocated by the destructor. 

### Example
```
#include "Wave.h"

int main()
{
	Wave sound(48000, 5);

	sound.Generate(880, SINE_WAVE);

	sound.Save("demodulated.wav");
	sound.Play();

	sound.Modulate(880, 10, 1.0, AMPLITUDE_MODULATION);

	sound.Save("modulated.wav");
	sound.Play();

	sound.Plot(1, 1);

	return 0;
}
```
Generates a `5` second `880` Hz `sine` wave with a sample rate of `48000` samples per second, saves it as `"demodulated.wav"`, and then plays it. Then, it creates a modulated waveform with a carrier frequency of `880` Hz, a modulation frequency of `10` Hz, and an index of `1.0` with the same sample rate and duration. It saves the new waveform as `"modulated.wav"`, plays it, and then plots `1` second with 100% quality (designated by the `1` which means use every sample, skipping none) which is shown below.

![Modulated Waveform](https://raw.githubusercontent.com/RobertDurfee/Wave/master/Modulated.png)
