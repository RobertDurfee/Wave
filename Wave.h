#ifndef WAVE_HEADER
#define WAVE_HEADER

#include <fstream>	//ifstream, ofstream
#include <Windows.h>	//PlaySound()
#include <sstream>	//stringstream

struct ResourceInterchangeFileFormatHeader
{
	unsigned long ID;
	unsigned long Size;
	unsigned long Format;
};

struct WaveFormatHeader
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

struct WaveDataHeader
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

#define SIZE_OF_HEADERS	0x2C

class Wave
{
public:
	Wave(ResourceInterchangeFileFormatHeader fileHeader, WaveFormatHeader formatHeader, WaveDataHeader dataHeader, char * waveform);
	Wave(ResourceInterchangeFileFormatHeader fileHeader, WaveFormatHeader formatHeader, WaveDataHeader dataHeader);
	Wave(int sampleRate, double duration, char * waveform);
	Wave(int sampleRate, double duration);
	Wave(char * filename);

	~Wave();

	void Play();
	void Plot(double duration, int quality);

	void Open(char * filename);
	void Save(char * filename);

	void Generate(double frequency, int type);

	void Modulate(double carrierFrequency, double modulationFrequency, double modulationIndex, int type);
	void Modulate(double carrierFrequency, char * modulationWaveform, double modulationIndex, int type);
	void Demodulate(double carrierFrequency, char * carrierWaveform, double modulationIndex, int type);

	void SetSampleRate(int sampleRate);
	int GetSampleRate();

	void SetDuration(double duration);
	double GetDuration();

	void SetWaveform(char * waveform);
	char * GetWaveform();

private:
	ResourceInterchangeFileFormatHeader fileHeader;
	WaveFormatHeader formatHeader;
	WaveDataHeader dataHeader;

	char * waveform;
	double duration;

	void SetHeaders(int samplerate, double duration);
	void SetHeaders(ResourceInterchangeFileFormatHeader fileHeader, WaveFormatHeader formatHeader, WaveDataHeader dataHeader);
	void SetHeaders(char * headers);
	char * GetHeaders();

	void ResizeWaveform();
};

Wave::Wave(ResourceInterchangeFileFormatHeader fileHeader, WaveFormatHeader formatHeader, WaveDataHeader dataHeader, char * waveform)
{
	SetHeaders(fileHeader, formatHeader, dataHeader);

	SetWaveform(waveform);
}
Wave::Wave(ResourceInterchangeFileFormatHeader fileHeader, WaveFormatHeader formatHeader, WaveDataHeader dataHeader)
{
	SetHeaders(fileHeader, formatHeader, dataHeader);

	ResizeWaveform();
}
Wave::Wave(int sampleRate, double duration, char * waveform)
{
	SetHeaders(sampleRate, duration);

	SetWaveform(waveform);
}
Wave::Wave(int sampleRate, double duration)
{
	SetHeaders(sampleRate, duration);

	ResizeWaveform();
}
Wave::Wave(char * filename)
{
	Open(filename);
}

Wave::~Wave()
{
	free(waveform);
}

void Wave::Play()
{
	char * wav = GetHeaders();
	wav = (char *)realloc(wav, SIZE_OF_HEADERS + (dataHeader.Size));
	memcpy(&wav[SIZE_OF_HEADERS], waveform, dataHeader.Size);

	PlaySound(wav, NULL, SND_MEMORY);

	free(wav);
}
void Wave::Plot(double duration, int quality)
{
	std::ofstream file("graph.dat");

	for (int i = 0; i < (int)(dataHeader.Size * (duration / this->duration)); i += quality)
		file << i << " " << (int)(waveform[i] & 0x000000FF) << std::endl;

	file.close();

	std::stringstream ss;
	ss << "gnuplot -e \"set terminal qt size 2000,1500; set yr [0:255]; set xr [0:" << (int)(dataHeader.Size * (duration / this->duration)) << "]; plot \'graph.dat\' with lines;\" -persist";

	system(ss.str().c_str());
}

void Wave::Open(char * filename)
{
	std::ifstream file(filename, std::ios::binary);

	char * headers = (char *)malloc(SIZE_OF_HEADERS);
	file.read(headers, SIZE_OF_HEADERS);
	SetHeaders(headers);
	free(headers);

	ResizeWaveform();

	file.read(waveform, dataHeader.Size);

	duration = dataHeader.Size / formatHeader.SampleRate;

	file.close();
}
void Wave::Save(char * filename)
{
	std::ofstream file(filename, std::ios::binary);

	char * headers = GetHeaders();
	file.write(headers, SIZE_OF_HEADERS);
	free(headers);

	file.write(waveform, dataHeader.Size);

	file.close();
}

void Wave::Generate(double frequency, int type)
{
	for (int t = 0; t < (int)dataHeader.Size; t++)
		if (type == SINE_WAVE)
			waveform[t] = (int)(127 * sin(2.0 * PI * frequency / formatHeader.SampleRate * t)) + 127;
		else if (type == COSINE_WAVE)
			waveform[t] = (int)(127 * cos(2.0 * PI * frequency / formatHeader.SampleRate * t)) + 127;
}

void Wave::Modulate(double carrierFrequency, double modulationFrequency, double modulationIndex, int type)
{
	for (int t = 0; t < (int)dataHeader.Size; t++)
		if (type == FREQUENCY_MODULATION)
			waveform[t] = (int)(127 * cos((2.0 * PI * carrierFrequency / formatHeader.SampleRate * t) - modulationIndex * sin(2.0 * PI * modulationFrequency / formatHeader.SampleRate * t))) + 127;
		else if (type == AMPLITUDE_MODULATION)
			waveform[t] = (int)((127 / (1 + modulationIndex)) * cos((2.0 * PI * carrierFrequency / formatHeader.SampleRate * t)) * (1 + modulationIndex * sin(2.0 * PI * modulationFrequency / formatHeader.SampleRate * t))) + 127;
}
void Wave::Modulate(double carrierFrequency, char * modulationWaveform, double modulationIndex, int type)
{
	for (int t = 0; t < (int)dataHeader.Size; t++)
		if (type == FREQUENCY_MODULATION)
			waveform[t] = (int)(127 * cos((2.0 * PI * carrierFrequency / formatHeader.SampleRate * t) - modulationIndex * (((modulationWaveform[t] & 0x000000FF) - 127.0) / 127.0))) + 127;
		else if (type == AMPLITUDE_MODULATION)
			waveform[t] = (int)((127 / (1 + modulationIndex)) * cos((2.0 * PI * carrierFrequency / formatHeader.SampleRate * t)) * (1 + modulationIndex * (((modulationWaveform[t] & 0x000000FF) - 127.0) / 127.0))) + 127;
}
void Wave::Demodulate(double carrierFrequency, char * carrierWaveform, double modulationIndex, int type)
{
	for (int t = 0; t < (int)dataHeader.Size; t++)
		if (type == AMPLITUDE_MODULATION)
			waveform[t] = (char)((((((carrierWaveform[t] & 0x000000FF) - 127.0) / 127.0) - 127.0) * (1 + modulationIndex)) / (127.0 * modulationIndex * cos(2.0 * PI * carrierFrequency / formatHeader.SampleRate * t)) - 1.0);
}

void Wave::SetSampleRate(int sampleRate)
{
	SetHeaders(sampleRate, dataHeader / formatHeader.SampleRate);

	ResizeWaveform();
}
int Wave::GetSampleRate()
{
	return formatHeader.SampleRate;
}

void Wave::SetDuration(double duration)
{
	SetHeaders(formatHeader.SampleRate, duration);
	
	ResizeWaveform();
}
double Wave::GetDuration()
{
	return duration;
}

void Wave::SetWaveform(char * waveform)
{
	ResizeWaveform();

	memcpy(this->waveform, waveform, dataHeader.Size);
}
char * Wave::GetWaveform()
{
	return waveform;
}

void Wave::SetHeaders(int sampleRate, double duration)
{
	fileHeader.ID = 0x46464952; // "RIFF"
	fileHeader.Format = 0x45564157; // "WAVE"

	formatHeader.ID = 0x20746D66; // "fmt "
	formatHeader.Size = 16;
	formatHeader.AudioFormat = 1;
	formatHeader.Channels = 1;
	formatHeader.SampleRate = sampleRate;
	formatHeader.ByteRate = formatHeader.SampleRate;
	formatHeader.BlockAlign = 1;
	formatHeader.BitsPerSample = 8;

	dataHeader.ID = 0x61746164; // "data"
	dataHeader.Size = (unsigned long)(formatHeader.SampleRate * duration);

	fileHeader.Size = 36 + dataHeader.Size;

	this->duration = duration;
}
void Wave::SetHeaders(ResourceInterchangeFileFormatHeader fileHeader, WaveFormatHeader formatHeader, WaveDataHeader dataHeader)
{
	this->fileHeader = fileHeader;
	this->formatHeader = formatHeader;
	this->dataHeader = dataHeader;
	duration = dataHeader.Size / formatHeader.SampleRate;
}
void Wave::SetHeaders(char * headers)
{
	int index = 0;

	//memcpy() is not used due to compiler-specfic structure padding
	fileHeader.ID = *(unsigned long *)&headers[index];			index += sizeof(unsigned long);
	fileHeader.Size = *(unsigned long *)&headers[index];			index += sizeof(unsigned long);
	fileHeader.Format = *(unsigned long *)&headers[index];			index += sizeof(unsigned long);

	formatHeader.ID = *(unsigned long *)&headers[index];			index += sizeof(unsigned long);
	formatHeader.Size = *(unsigned long *)&headers[index];			index += sizeof(unsigned long);
	formatHeader.AudioFormat = *(unsigned short *)&headers[index];		index += sizeof(unsigned short);
	formatHeader.Channels = *(unsigned short *)&headers[index];		index += sizeof(unsigned short);
	formatHeader.SampleRate = *(unsigned long *)&headers[index];		index += sizeof(unsigned long);
	formatHeader.ByteRate = *(unsigned long *)& headers[index];		index += sizeof(unsigned long);
	formatHeader.BlockAlign = *(unsigned short *)&headers[index];		index += sizeof(unsigned short);
	formatHeader.BitsPerSample = *(unsigned short *)&headers[index];	index += sizeof(unsigned short);

	dataHeader.ID = *(unsigned long *)&headers[index];			index += sizeof(unsigned long);
	dataHeader.Size = *(unsigned long *)&headers[index];
}
char * Wave::GetHeaders()
{
	char * headers = (char *)malloc(SIZE_OF_HEADERS);

	int index = 0;

	//memcpy() is not used due to compiler-specfic structure padding
	*(unsigned long*)&headers[index] = fileHeader.ID;			index += sizeof(unsigned long);
	*(unsigned long*)&headers[index] = fileHeader.Size;			index += sizeof(unsigned long);
	*(unsigned long*)&headers[index] = fileHeader.Format;			index += sizeof(unsigned long);

	*(unsigned long*)&headers[index] = formatHeader.ID;			index += sizeof(unsigned long);
	*(unsigned long*)&headers[index] = formatHeader.Size;			index += sizeof(unsigned long);
	*(unsigned short*)&headers[index] = formatHeader.AudioFormat;		index += sizeof(unsigned short);
	*(unsigned short*)&headers[index] = formatHeader.Channels;		index += sizeof(unsigned short);
	*(unsigned long*)&headers[index] = formatHeader.SampleRate;		index += sizeof(unsigned long);
	*(unsigned long*)&headers[index] = formatHeader.ByteRate;		index += sizeof(unsigned long);
	*(unsigned short*)&headers[index] = formatHeader.BlockAlign;		index += sizeof(unsigned short);
	*(unsigned short*)&headers[index] = formatHeader.BitsPerSample;		index += sizeof(unsigned short);

	*(unsigned long*)&headers[index] = dataHeader.ID;			index += sizeof(unsigned long);
	*(unsigned long*)&headers[index] = dataHeader.Size;

	return headers;
}

void Wave::ResizeWaveform()
{
	if (waveform)
		realloc(waveform, dataHeader.Size);
	else
		waveform = (char *)malloc(dataHeader.Size);
}

#endif
