#ifndef WAV_HEADER
#define WAV_HEADER

#include <fstream>		//ifstream, ofstream
#include <Windows.h>	//PlaySound()
#include <sstream>		//stringstream

struct RIFF_FILE_HEADER
{
	unsigned long ID;
	unsigned long Size;
	unsigned long Format;
};

struct WAV_FORMAT_HEADER
{
	unsigned long ID;
	unsigned long Size;
	unsigned short AudioFormat;
	unsigned short Channels;
	unsigned long SampleRate;
	unsigned long ByteRate;
	unsigned short BlockAlign;
	unsigned short BitsPerSample;
};

struct WAV_DATA_HEADER
{
	unsigned long ID;
	unsigned long Size;
};

#ifndef PI
#define PI 3.14159265
#endif

#define SINE_WAVE   0x1
#define COSINE_WAVE 0x2
#define SQUARE_WAVE 0x3

#define FREQUENCY_MODULATION 0x1
#define AMPLITUDE_MODULATION 0x2

class Wave
{
public:
	Wave(RIFF_FILE_HEADER fileHeader, WAV_FORMAT_HEADER formatHeader, WAV_DATA_HEADER dataHeader, char * waveform)
	{
		this->fileHeader = fileHeader;
		this->formatHeader = formatHeader;
		this->dataHeader = dataHeader;
		this->duration = dataHeader.Size / formatHeader.SampleRate;
		this->waveform = waveform;
		this->createdWaveform = false;
	}
	Wave(RIFF_FILE_HEADER fileHeader, WAV_FORMAT_HEADER formatHeader, WAV_DATA_HEADER dataHeader)
	{
		this->fileHeader = fileHeader;
		this->formatHeader = formatHeader;
		this->dataHeader = dataHeader;
		this->duration = dataHeader.Size / formatHeader.SampleRate;
		this->waveform = (char *)malloc(dataHeader.Size);
		this->createdWaveform = true;
	}
	Wave(int sampleRate, double duration, char * waveform)
	{
		this->fileHeader.ID = 0x46464952; // "RIFF"
		this->fileHeader.Format = 0x45564157; // "WAVE"

		this->formatHeader.ID = 0x20746D66; // "fmt "
		this->formatHeader.Size = 16;
		this->formatHeader.AudioFormat = 1;
		this->formatHeader.Channels = 1;
		this->formatHeader.SampleRate = sampleRate;
		this->formatHeader.ByteRate = this->formatHeader.SampleRate;
		this->formatHeader.BlockAlign = 1;
		this->formatHeader.BitsPerSample = 8;

		this->dataHeader.ID = 0x61746164; // "data"
		this->dataHeader.Size = this->formatHeader.SampleRate * duration;

		this->fileHeader.Size = 36 + this->dataHeader.Size;

		this->duration = duration;

		this->waveform = waveform;
		this->createdWaveform = false;
	}
	Wave(int sampleRate, double duration)
	{
		this->fileHeader.ID = 0x46464952; // "RIFF"
		this->fileHeader.Format = 0x45564157; // "WAVE"

		this->formatHeader.ID = 0x20746D66; // "fmt "
		this->formatHeader.Size = 16;
		this->formatHeader.AudioFormat = 1;
		this->formatHeader.Channels = 1;
		this->formatHeader.SampleRate = sampleRate;
		this->formatHeader.ByteRate = this->formatHeader.SampleRate;
		this->formatHeader.BlockAlign = 1;
		this->formatHeader.BitsPerSample = 8;

		this->dataHeader.ID = 0x61746164; // "data"
		this->dataHeader.Size = this->formatHeader.SampleRate * duration;

		this->fileHeader.Size = 36 + this->dataHeader.Size;

		this->duration = duration;

		this->waveform = (char *)malloc(this->dataHeader.Size);
		this->createdWaveform = true;
	}
	Wave(char * filename)
	{
		this->Open(filename);
	}
	~Wave()
	{
		if (this->createdWaveform)
			free(this->waveform);
	}
	void Play()
	{
		char * wav = (char *)malloc(sizeof(RIFF_FILE_HEADER) + sizeof(WAV_FORMAT_HEADER) + sizeof(WAV_DATA_HEADER) + this->dataHeader.Size);
		memcpy(&wav[0], &this->fileHeader, sizeof(RIFF_FILE_HEADER));
		memcpy(&wav[sizeof(RIFF_FILE_HEADER)], &this->formatHeader, sizeof(WAV_FORMAT_HEADER));
		memcpy(&wav[sizeof(RIFF_FILE_HEADER) + sizeof(WAV_FORMAT_HEADER)], &this->dataHeader, sizeof(WAV_DATA_HEADER));
		memcpy(&wav[sizeof(RIFF_FILE_HEADER) + sizeof(WAV_FORMAT_HEADER) + sizeof(WAV_DATA_HEADER)], this->waveform, this->dataHeader.Size);

		PlaySound(wav, NULL, SND_MEMORY);

		free(wav);
	}
	void Save(char * filename)
	{
		std::ofstream file(filename, std::ios::binary);
		
		file.write((char *)&this->fileHeader.ID, sizeof(unsigned long));
		file.write((char *)&this->fileHeader.Size, sizeof(unsigned long));
		file.write((char *)&this->fileHeader.Format, sizeof(unsigned long));
		
		file.write((char *)&this->formatHeader.ID, sizeof(unsigned long));
		file.write((char *)&this->formatHeader.Size, sizeof(unsigned long));
		file.write((char *)&this->formatHeader.AudioFormat, sizeof(unsigned short));
		file.write((char *)&this->formatHeader.Channels, sizeof(unsigned short));
		file.write((char *)&this->formatHeader.SampleRate, sizeof(unsigned long));
		file.write((char *)&this->formatHeader.ByteRate, sizeof(unsigned long));
		file.write((char *)&this->formatHeader.BlockAlign, sizeof(unsigned short));
		file.write((char *)&this->formatHeader.BitsPerSample, sizeof(unsigned short));
		
		file.write((char *)&this->dataHeader.ID, sizeof(unsigned long));
		file.write((char *)&this->dataHeader.Size, sizeof(unsigned long));

		file.write(this->waveform, this->dataHeader.Size);

		file.close();
	}
	void Open(char * filename)
	{
		if (this->createdWaveform)
			free(this->waveform);

		std::ifstream file(filename, std::ios::binary);

		file.read((char *)&this->fileHeader.ID, sizeof(unsigned long));
		file.read((char *)&this->fileHeader.Size, sizeof(unsigned long));
		file.read((char *)&this->fileHeader.Format, sizeof(unsigned long));

		file.read((char *)&this->formatHeader.ID, sizeof(unsigned long));
		file.read((char *)&this->formatHeader.Size, sizeof(unsigned long));
		file.read((char *)&this->formatHeader.AudioFormat, sizeof(unsigned short));
		file.read((char *)&this->formatHeader.Channels, sizeof(unsigned short));
		file.read((char *)&this->formatHeader.SampleRate, sizeof(unsigned long));
		file.read((char *)&this->formatHeader.ByteRate, sizeof(unsigned long));
		file.read((char *)&this->formatHeader.BlockAlign, sizeof(unsigned short));
		file.read((char *)&this->formatHeader.BitsPerSample, sizeof(unsigned short));

		file.read((char *)&this->dataHeader.ID, sizeof(unsigned long));
		file.read((char *)&this->dataHeader.Size, sizeof(unsigned long));

		this->waveform = (char *)malloc(this->dataHeader.Size);
		this->createdWaveform = true;

		file.read(this->waveform, this->dataHeader.Size);

		this->duration = this->dataHeader.Size / this->formatHeader.SampleRate;

		file.close();
	}
	void Generate(double frequency, int type)
	{
		for (int t = 0; t < (int)this->dataHeader.Size; t++)
			if (type == SINE_WAVE)
				this->waveform[t] = (int)(127 * sin(2.0 * PI * frequency / this->formatHeader.SampleRate * t)) + 127;
			else if (type == COSINE_WAVE)
				this->waveform[t] = (int)(127 * cos(2.0 * PI * frequency / this->formatHeader.SampleRate * t)) + 127;
	}
	void Modulate(double carrierFrequency, double modulationFrequency, double modulationIndex, int type)
	{
		for (int t = 0; t < (int)this->dataHeader.Size; t++)
			if (type == FREQUENCY_MODULATION)
				this->waveform[t] = (int)(127 * cos((2.0 * PI * carrierFrequency / this->formatHeader.SampleRate * t) - modulationIndex * sin(2.0 * PI * modulationFrequency / this->formatHeader.SampleRate * t))) + 127;
			else if (type == AMPLITUDE_MODULATION)
				this->waveform[t] = (int)((127 / (1 + modulationIndex)) * cos((2.0 * PI * carrierFrequency / this->formatHeader.SampleRate * t)) * (1 + modulationIndex * sin(2.0 * PI * modulationFrequency / this->formatHeader.SampleRate * t))) + 127;
	}
	void Modulate(double carrierFrequency, char * modulationWaveform, double modulationIndex, int type)
	{
		for (int t = 0; t < (int)this->dataHeader.Size; t++)
			if (type == FREQUENCY_MODULATION)
				this->waveform[t] = (int)(127 * cos((2.0 * PI * carrierFrequency / this->formatHeader.SampleRate * t) - modulationIndex * (((modulationWaveform[t] & 0x000000FF) - 127.0) / 127.0))) + 127;
			else if (type == AMPLITUDE_MODULATION)
				this->waveform[t] = (int)((127 / (1 + modulationIndex)) * cos((2.0 * PI * carrierFrequency / this->formatHeader.SampleRate * t)) * (1 + modulationIndex * (((modulationWaveform[t] & 0x000000FF) - 127.0) / 127.0))) + 127;
	}
	void Demodulate(double carrierFrequency, char * carrierWaveform, double modulationIndex, int type)
	{
		for (int t = 0; t < (int)this->dataHeader.Size; t++)
			if (type == AMPLITUDE_MODULATION)
				this->waveform[t] = (((((carrierWaveform[t] & 0x000000FF) - 127.0) / 127.0) - 127.0) * (1 + modulationIndex)) / (127.0 * modulationIndex * cos(2.0 * PI * carrierFrequency / this->formatHeader.SampleRate * t)) - 1.0;
	}
	void Plot(double duration, int quality)
	{
		std::ofstream file("graph.dat");

		for (int i = 0; i < (int)(this->dataHeader.Size * (duration / this->duration)); i += quality)
			file << i << " " << (int)(this->waveform[i] & 0x000000FF) << std::endl;

		file.close();

		std::stringstream ss;
		ss << "gnuplot -e \"set terminal qt size 2000,1500; set yr [0:255]; set xr [0:" << (int)(this->dataHeader.Size * (duration / this->duration)) << "]; plot \'graph.dat\' with lines;\" -persist";

		system(ss.str().c_str());
	}
	void SetSampleRate(int sampleRate)
	{
		this->formatHeader.SampleRate = sampleRate;
		this->formatHeader.ByteRate = this->formatHeader.SampleRate;
		this->dataHeader.Size = this->formatHeader.SampleRate * this->duration;

		this->fileHeader.Size = 36 + this->dataHeader.Size;

		if (this->createdWaveform)
			free(this->waveform);

		this->waveform = (char *)malloc(this->dataHeader.Size);
		this->createdWaveform = true;
	}
	int GetSampleRate()
	{
		return this->formatHeader.SampleRate;
	}
	void SetDuration(int duration)
	{
		this->duration = duration;
		this->dataHeader.Size = this->formatHeader.SampleRate * duration;

		this->fileHeader.Size = 36 + this->dataHeader.Size;

		if (this->createdWaveform)
			free(this->waveform);

		this->waveform = (char *)malloc(this->dataHeader.Size);
		this->createdWaveform = true;
	}
	int GetDuration()
	{
		return this->duration;
	}
	char * GetWaveform()
	{
		return this->waveform;
	}
private:
	RIFF_FILE_HEADER fileHeader;
	WAV_FORMAT_HEADER formatHeader;
	WAV_DATA_HEADER dataHeader;
	char * waveform;
	bool createdWaveform;
	double duration;
};

#endif