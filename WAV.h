#ifndef WAV_HEADER
#define WAV_HEADER

#include <fstream>	//ifstream, ofstream
#include <Windows.h>	//PlaySound()
#include <sstream>	//stringstream

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

#define SIZE_OF_HEADERS	0x2C

class Wave
{
public:
	Wave(RIFF_FILE_HEADER fileHeader, WAV_FORMAT_HEADER formatHeader, WAV_DATA_HEADER dataHeader, char * waveform);
	Wave(RIFF_FILE_HEADER fileHeader, WAV_FORMAT_HEADER formatHeader, WAV_DATA_HEADER dataHeader);
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
	RIFF_FILE_HEADER fileHeader;
	WAV_FORMAT_HEADER formatHeader;
	WAV_DATA_HEADER dataHeader;

	char * waveform;
	bool createdWaveform;
	double duration;

	void SetHeaders(int samplerate, double duration);
	void SetHeaders(RIFF_FILE_HEADER fileHeader, WAV_FORMAT_HEADER formatHeader, WAV_DATA_HEADER dataHeader);
	void SetHeaders(char * headers);
	char * GetHeaders();
};

Wave::Wave(RIFF_FILE_HEADER fileHeader, WAV_FORMAT_HEADER formatHeader, WAV_DATA_HEADER dataHeader, char * waveform)
{
	this->SetHeaders(fileHeader, formatHeader, dataHeader);

	this->waveform = waveform;
	this->createdWaveform = false;
}
Wave::Wave(RIFF_FILE_HEADER fileHeader, WAV_FORMAT_HEADER formatHeader, WAV_DATA_HEADER dataHeader)
{
	this->SetHeaders(fileHeader, formatHeader, dataHeader);

	this->waveform = (char *)malloc(dataHeader.Size * sizeof(char));
	this->createdWaveform = true;
}
Wave::Wave(int sampleRate, double duration, char * waveform)
{
	this->SetHeaders(sampleRate, duration);

	this->waveform = waveform;
	this->createdWaveform = false;
}
Wave::Wave(int sampleRate, double duration)
{
	this->SetHeaders(sampleRate, duration);

	this->waveform = (char *)malloc(this->dataHeader.Size * sizeof(char));
	this->createdWaveform = true;
}
Wave::Wave(char * filename)
{
	this->Open(filename);
}

Wave::~Wave()
{
	if (this->createdWaveform)
		free(this->waveform);
}

void Wave::Play()
{
	char * wav = this->GetHeaders();
	wav = (char *)realloc(wav, SIZE_OF_HEADERS + (this->dataHeader.Size * sizeof(char)));
	memcpy(&wav[SIZE_OF_HEADERS], this->waveform, this->dataHeader.Size * sizeof(char));

	PlaySound(wav, NULL, SND_MEMORY);

	free(wav);
}
void Wave::Plot(double duration, int quality)
{
	std::ofstream file("graph.dat");

	for (int i = 0; i < (int)(this->dataHeader.Size * (duration / this->duration)); i += quality)
		file << i << " " << (int)(this->waveform[i] & 0x000000FF) << std::endl;

	file.close();

	std::stringstream ss;
	ss << "gnuplot -e \"set terminal qt size 2000,1500; set yr [0:255]; set xr [0:" << (int)(this->dataHeader.Size * (duration / this->duration)) << "]; plot \'graph.dat\' with lines;\" -persist";

	system(ss.str().c_str());
}

void Wave::Open(char * filename)
{
	if (this->createdWaveform)
		free(this->waveform);

	std::ifstream file(filename, std::ios::binary);

	char headers[SIZE_OF_HEADERS];
	file.read(headers, SIZE_OF_HEADERS);
	this->SetHeaders(headers);

	this->waveform = (char *)malloc(this->dataHeader.Size * sizeof(char));
	this->createdWaveform = true;

	file.read(this->waveform, this->dataHeader.Size);

	this->duration = this->dataHeader.Size / this->formatHeader.SampleRate;

	file.close();
}
void Wave::Save(char * filename)
{
	std::ofstream file(filename, std::ios::binary);

	char * headers = this->GetHeaders();
	file.write(headers, SIZE_OF_HEADERS);
	free(headers);

	file.write(this->waveform, this->dataHeader.Size);

	file.close();
}

void Wave::Generate(double frequency, int type)
{
	for (int t = 0; t < (int)this->dataHeader.Size; t++)
		if (type == SINE_WAVE)
			this->waveform[t] = (int)(127 * sin(2.0 * PI * frequency / this->formatHeader.SampleRate * t)) + 127;
		else if (type == COSINE_WAVE)
			this->waveform[t] = (int)(127 * cos(2.0 * PI * frequency / this->formatHeader.SampleRate * t)) + 127;
}

void Wave::Modulate(double carrierFrequency, double modulationFrequency, double modulationIndex, int type)
{
	for (int t = 0; t < (int)this->dataHeader.Size; t++)
		if (type == FREQUENCY_MODULATION)
			this->waveform[t] = (int)(127 * cos((2.0 * PI * carrierFrequency / this->formatHeader.SampleRate * t) - modulationIndex * sin(2.0 * PI * modulationFrequency / this->formatHeader.SampleRate * t))) + 127;
		else if (type == AMPLITUDE_MODULATION)
			this->waveform[t] = (int)((127 / (1 + modulationIndex)) * cos((2.0 * PI * carrierFrequency / this->formatHeader.SampleRate * t)) * (1 + modulationIndex * sin(2.0 * PI * modulationFrequency / this->formatHeader.SampleRate * t))) + 127;
}
void Wave::Modulate(double carrierFrequency, char * modulationWaveform, double modulationIndex, int type)
{
	for (int t = 0; t < (int)this->dataHeader.Size; t++)
		if (type == FREQUENCY_MODULATION)
			this->waveform[t] = (int)(127 * cos((2.0 * PI * carrierFrequency / this->formatHeader.SampleRate * t) - modulationIndex * (((modulationWaveform[t] & 0x000000FF) - 127.0) / 127.0))) + 127;
		else if (type == AMPLITUDE_MODULATION)
			this->waveform[t] = (int)((127 / (1 + modulationIndex)) * cos((2.0 * PI * carrierFrequency / this->formatHeader.SampleRate * t)) * (1 + modulationIndex * (((modulationWaveform[t] & 0x000000FF) - 127.0) / 127.0))) + 127;
}
void Wave::Demodulate(double carrierFrequency, char * carrierWaveform, double modulationIndex, int type)
{
	for (int t = 0; t < (int)this->dataHeader.Size; t++)
		if (type == AMPLITUDE_MODULATION)
			this->waveform[t] = (char)((((((carrierWaveform[t] & 0x000000FF) - 127.0) / 127.0) - 127.0) * (1 + modulationIndex)) / (127.0 * modulationIndex * cos(2.0 * PI * carrierFrequency / this->formatHeader.SampleRate * t)) - 1.0);
}

void Wave::SetSampleRate(int sampleRate)
{
	this->formatHeader.SampleRate = sampleRate;
	this->formatHeader.ByteRate = this->formatHeader.SampleRate;
	this->dataHeader.Size = (unsigned long)(this->formatHeader.SampleRate * this->duration);

	this->fileHeader.Size = 36 + this->dataHeader.Size;

	if (this->createdWaveform)
		free(this->waveform);

	this->waveform = (char *)malloc(this->dataHeader.Size * sizeof(char));
	this->createdWaveform = true;
}
int Wave::GetSampleRate()
{
	return this->formatHeader.SampleRate;
}

void Wave::SetDuration(double duration)
{
	this->duration = duration;
	this->dataHeader.Size = (unsigned long)(this->formatHeader.SampleRate * duration);

	this->fileHeader.Size = 36 + this->dataHeader.Size;

	if (this->createdWaveform)
		free(this->waveform);

	this->waveform = (char *)malloc(this->dataHeader.Size * sizeof(char));
	this->createdWaveform = true;
}
double Wave::GetDuration()
{
	return this->duration;
}

void Wave::SetWaveform(char * waveform)
{
	if (this->createdWaveform)
		free(this->waveform);

	this->createdWaveform = false;

	this->waveform = waveform;
}
char * Wave::GetWaveform()
{
	return this->waveform;
}

void Wave::SetHeaders(int sampleRate, double duration)
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
	this->dataHeader.Size = (unsigned long)(this->formatHeader.SampleRate * duration);

	this->fileHeader.Size = 36 + this->dataHeader.Size;

	this->duration = duration;
}
void Wave::SetHeaders(RIFF_FILE_HEADER fileHeader, WAV_FORMAT_HEADER formatHeader, WAV_DATA_HEADER dataHeader)
{
	this->fileHeader = fileHeader;
	this->formatHeader = formatHeader;
	this->dataHeader = dataHeader;
	this->duration = dataHeader.Size / formatHeader.SampleRate;
}
void Wave::SetHeaders(char * headers)
{
	int index = 0;

	this->fileHeader.ID = *(unsigned long *)&headers[index]; index += sizeof(unsigned long);
	this->fileHeader.Size = *(unsigned long *)&headers[index]; index += sizeof(unsigned long);
	this->fileHeader.Format = *(unsigned long *)&headers[index]; index += sizeof(unsigned long);

	this->formatHeader.ID = *(unsigned long *)&headers[index]; index += sizeof(unsigned long);
	this->formatHeader.Size = *(unsigned long *)&headers[index]; index += sizeof(unsigned long);
	this->formatHeader.AudioFormat = *(unsigned short *)&headers[index]; index += sizeof(unsigned short);
	this->formatHeader.Channels = *(unsigned short *)&headers[index]; index += sizeof(unsigned short);
	this->formatHeader.SampleRate = *(unsigned long *)&headers[index]; index += sizeof(unsigned long);
	this->formatHeader.ByteRate = *(unsigned long *)& headers[index]; index += sizeof(unsigned long);
	this->formatHeader.BlockAlign = *(unsigned short *)&headers[index]; index += sizeof(unsigned short);
	this->formatHeader.BitsPerSample = *(unsigned short *)&headers[index]; index += sizeof(unsigned short);

	this->dataHeader.ID = *(unsigned long *)&headers[index]; index += sizeof(unsigned long);
	this->dataHeader.Size = *(unsigned long *)&headers[index];
}
char * Wave::GetHeaders()
{
	char * headers = (char *)malloc(SIZE_OF_HEADERS);

	int index = 0;

	*(unsigned long*)&headers[index] = this->fileHeader.ID; index += sizeof(unsigned long);
	*(unsigned long*)&headers[index] = this->fileHeader.Size; index += sizeof(unsigned long);
	*(unsigned long*)&headers[index] = this->fileHeader.Format; index += sizeof(unsigned long);

	*(unsigned long*)&headers[index] = this->formatHeader.ID; index += sizeof(unsigned long);
	*(unsigned long*)&headers[index] = this->formatHeader.Size; index += sizeof(unsigned long);
	*(unsigned short*)&headers[index] = this->formatHeader.AudioFormat; index += sizeof(unsigned short);
	*(unsigned short*)&headers[index] = this->formatHeader.Channels; index += sizeof(unsigned short);
	*(unsigned long*)&headers[index] = this->formatHeader.SampleRate; index += sizeof(unsigned long);
	*(unsigned long*)&headers[index] = this->formatHeader.ByteRate; index += sizeof(unsigned long);
	*(unsigned short*)&headers[index] = this->formatHeader.BlockAlign; index += sizeof(unsigned short);
	*(unsigned short*)&headers[index] = this->formatHeader.BitsPerSample; index += sizeof(unsigned short);

	*(unsigned long*)&headers[index] = this->dataHeader.ID; index += sizeof(unsigned long);
	*(unsigned long*)&headers[index] = this->dataHeader.Size;

	return headers;
}

#endif
