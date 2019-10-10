
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "WAVheader.h"

#define BLOCK_SIZE 16
#define MAX_NUM_CHANNEL 8
#define M_PI 3.14159265358979323846

double sampleBuffer[MAX_NUM_CHANNEL][BLOCK_SIZE];
double leviTok[BLOCK_SIZE];
double desniTok[BLOCK_SIZE];

double coefs[4] = { 0, 0, 0, 0 };

double* coefs_ptr = coefs;

void getCoefs(double sampling_freq, double cut_freq)
{
	

	*(coefs + 3) = (1 - sin(2 * M_PI * cut_freq / sampling_freq)) / cos(2 * M_PI * cut_freq / sampling_freq);
	if (*(coefs + 3) < -1 || *(coefs) > 1)
	{
		*(coefs + 3) = (1 + sin(2 * M_PI * cut_freq / sampling_freq)) / cos(2 * M_PI * cut_freq / sampling_freq);
	}

	*(coefs) = *(coefs + 1) = (1 - *(coefs + 3)) / 2;

	*(coefs + 2) = 1;

	*(coefs + 3) = -*(coefs + 3);
}

double CLIP(double accum) {
	if (accum > 1) {
		accum = 1;
	}
	else if (accum < -1) {
		accum = -1;
	}

	return accum;
}

double first_order_IIR(double input, double* coefficients, double* z_x, double* z_y)
{
	double temp;

	coefs_ptr = coefs;

	z_x[0] = input; /* Copy input to x[0] */

	temp = (*coefficients * *(z_x));   /* B0 * x(n)     */
	temp += (*(coefficients + 1) * *(z_x + 1));    /* B1 * x(n-1) */
	temp -= (*(coefficients + 3)* *(z_y + 1));    /* A1 * y(n-1) */


	*z_y = (temp);

	/* Shuffle values along one place for next time */

	*(z_y + 1) = *(z_y);   /* y(n-1) = y(n)   */
	*(z_x + 1) = *(z_x);   /* x(n-1) = x(n)   */

	return (temp);
}
double shelvingLP(double input, double* coeff, double* z_x, double* z_y, double k)
{
	double filtered_input, output;
	double accum;
	filtered_input = first_order_IIR(input, coeff, z_x, z_y);
	accum = (input + filtered_input) / 2.0;
	accum += ((input - filtered_input) / 2.0)*k;
	output = CLIP(accum);
	return output;
}



double x_history0[] = { 0,0 };

double y_history0[] = { 0,0 };


void processing() {

	for (int i = 0; i < BLOCK_SIZE; i++) {
		sampleBuffer[0][i] = shelvingLP(sampleBuffer[0][i], coefs, x_history0, y_history0, 16);
	}
}


int main(int argc, char* argv[])
{
	FILE *wav_in = NULL;
	FILE *wav_out = NULL;
	char WavInputName[256];
	char WavOutputName[256];
	WAV_HEADER inputWAVhdr, outputWAVhdr;

	// Init channel buffers
	for (int i = 0; i < MAX_NUM_CHANNEL; i++)
		memset(&sampleBuffer[i], 0, BLOCK_SIZE);

	// Open input and output wav files
	//-------------------------------------------------
	strcpy(WavInputName, argv[1]);
	wav_in = OpenWavFileForRead(WavInputName, "rb");
	strcpy(WavOutputName, argv[2]);
	wav_out = OpenWavFileForRead(WavOutputName, "wb");
	//-------------------------------------------------

	double sampleFreq = atof(argv[3]);

	double cutFreq = atof(argv[4]);
	// Read input wav header
	//-------------------------------------------------
	ReadWavHeader(wav_in, inputWAVhdr);
	//-------------------------------------------------

	// Set up output WAV header
	//-------------------------------------------------	
	outputWAVhdr = inputWAVhdr;
	outputWAVhdr.fmt.NumChannels = inputWAVhdr.fmt.NumChannels; // change number of channels

	int oneChannelSubChunk2Size = inputWAVhdr.data.SubChunk2Size / inputWAVhdr.fmt.NumChannels;
	int oneChannelByteRate = inputWAVhdr.fmt.ByteRate / inputWAVhdr.fmt.NumChannels;
	int oneChannelBlockAlign = inputWAVhdr.fmt.BlockAlign / inputWAVhdr.fmt.NumChannels;

	outputWAVhdr.data.SubChunk2Size = oneChannelSubChunk2Size * outputWAVhdr.fmt.NumChannels;
	outputWAVhdr.fmt.ByteRate = oneChannelByteRate * outputWAVhdr.fmt.NumChannels;
	outputWAVhdr.fmt.BlockAlign = oneChannelBlockAlign * outputWAVhdr.fmt.NumChannels;


	// Write output WAV header to file
	//-------------------------------------------------
	WriteWavHeader(wav_out, outputWAVhdr);

	getCoefs(sampleFreq, cutFreq);

	// Processing loop
	//-------------------------------------------------	
	{
		int sample;
		int BytesPerSample = inputWAVhdr.fmt.BitsPerSample / 8;
		const double SAMPLE_SCALE = -(double)(1 << 31);		//2^31
		int iNumSamples = inputWAVhdr.data.SubChunk2Size / (inputWAVhdr.fmt.NumChannels*inputWAVhdr.fmt.BitsPerSample / 8);



		// exact file length should be handled correctly...
		for (int i = 0; i < iNumSamples / BLOCK_SIZE; i++)
		{
			for (int j = 0; j < BLOCK_SIZE; j++)
			{
				for (int k = 0; k < inputWAVhdr.fmt.NumChannels; k++)
				{
					sample = 0; //debug
					fread(&sample, BytesPerSample, 1, wav_in);
					sample = sample << (32 - inputWAVhdr.fmt.BitsPerSample); // force signextend
					sampleBuffer[k][j] = sample / SAMPLE_SCALE;				// scale sample to 1.0/-1.0 range		
				}
			}


			processing();

			for (int j = 0; j < BLOCK_SIZE; j++)
			{
				for (int k = 0; k < outputWAVhdr.fmt.NumChannels; k++)
				{
					sample = sampleBuffer[k][j] * SAMPLE_SCALE;	// crude, non-rounding 			
					sample = sample >> (32 - inputWAVhdr.fmt.BitsPerSample);
					fwrite(&sample, outputWAVhdr.fmt.BitsPerSample / 8, 1, wav_out);
				}
			}
		}
	}




	// Close files
	//-------------------------------------------------	
	fclose(wav_in);
	fclose(wav_out);
	//-------------------------------------------------	

	return 0;
}