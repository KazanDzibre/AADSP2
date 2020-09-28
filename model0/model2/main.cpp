#include <stdlib.h>
#include <string.h>
#include "WAVheader.h"
#include "common.h"

// Buffer
DSPfract sampleBuffer[MAX_NUM_CHANNEL][BLOCK_SIZE];
DSPfract coefs[4] = { FRACT_NUM(0.0), FRACT_NUM(0.0), FRACT_NUM(0.0), FRACT_NUM(0.0) };

DSPfract CLIP(DSPfract accum) {
	if (accum > FRACT_NUM(1.0)) {
		accum = FRACT_NUM(1.0);
	}
	else if (accum < FRACT_NUM(-1.0)) {
		accum = FRACT_NUM(-1.0);
	}

	return accum;
}

void getCoefs(DSPfract alpha, DSPfract* coefs)
{
	DSPfract tmp1, tmp2;

	tmp1 = FRACT_NUM(1.0 * alpha);
	tmp1 = CLIP(tmp1);

	tmp2 = -FRACT_NUM(1.0 * alpha);
	tmp2 = CLIP(tmp2);

	coefs[0] = tmp1;
	coefs[1] = -FRACT_NUM(1.0);
	coefs[2] = FRACT_NUM(1.0);
	coefs[3] = tmp2;
}

DSPfract getAlpha(DSPfract w)
{
	DSPfract tmp1 = FRACT_NUM(1 / cos(w) + tan(w));
	DSPfract tmp2 = FRACT_NUM(1 / cos(w) - tan(w));

	return (tmp1 >= FRACT_NUM(-1.0) && tmp1 <= FRACT_NUM(1.0)) ? tmp1 : tmp2;
}

DSPfract first_order_IIR(DSPfract input, DSPfract* coefficients, DSPfract* z_x, DSPfract* z_y)
{
	DSPfract temp;

	z_x[0] = input; /* Copy input to x[0] */

	temp = (coefficients[0] * z_x[0]);   /* B0 * x(n)     */
	temp += (coefficients[1] * z_x[1]);    /* B1 * x(n-1) */
	temp -= (coefficients[3] * z_y[1]);    /* A1 * y(n-1) */


	z_y[0] = (temp);

	/* Shuffle values along one place for next time */

	z_y[1] = z_y[0];   /* y(n-1) = y(n)   */
	z_x[1] = z_x[0];   /* x(n-1) = x(n)   */

	return (temp);
}

DSPfract shelvingLP(DSPfract input, DSPfract* coeff, DSPfract* z_x, DSPfract* z_y, DSPfract k) {

	DSPfract filtered_input;
	DSPfract accum;

	filtered_input = first_order_IIR(input, coeff, z_x, z_y);
	accum = (input + filtered_input) / FRACT_NUM(2.0);
	accum += ((input - filtered_input) / FRACT_NUM(2.0)) * k;
	accum = CLIP(accum);


	return accum;

}

DSPfract x_history0[] = { FRACT_NUM(0.0), FRACT_NUM(0.0) };

DSPfract y_history0[] = { FRACT_NUM(0.0), FRACT_NUM(0.0) };

void processing() {
	for (DSPint i = 0; i < BLOCK_SIZE; i++) {
		sampleBuffer[0][i] = shelvingLP(sampleBuffer[0][i], coefs, x_history0, y_history0, 1.2);
	}
}

DSPint main(DSPint argc, char* argv[])
{
	FILE* wav_in = NULL;
	FILE* wav_out = NULL;
	char WavInputName[256];
	char WavOutputName[256];
	WAV_HEADER inputWAVhdr, outputWAVhdr;

	// Init channel buffers
	for (DSPint i = 0; i < MAX_NUM_CHANNEL; i++)
		for (DSPint j = 0; j < BLOCK_SIZE; j++)
			sampleBuffer[i][j] = FRACT_NUM(0.0);

	DSPfract fc = atof(argv[3]);
	// Open input and output wav files
	//-------------------------------------------------
	strcpy(WavInputName, argv[1]);
	wav_in = OpenWavFileForRead(WavInputName, "rb");
	strcpy(WavOutputName, argv[2]);
	wav_out = OpenWavFileForRead(WavOutputName, "wb");
	//-------------------------------------------------

	// Read input wav header
	//-------------------------------------------------
	ReadWavHeader(wav_in, inputWAVhdr);
	//-------------------------------------------------

	// Set up output WAV header
	//-------------------------------------------------	
	outputWAVhdr = inputWAVhdr;
	//outputWAVhdr.fmt.NumChannels = inputWAVhdr.fmt.NumChannels; // change number of channels
	outputWAVhdr.fmt.NumChannels = MAX_NUM_CHANNEL;

	DSPint oneChannelSubChunk2Size = inputWAVhdr.data.SubChunk2Size / inputWAVhdr.fmt.NumChannels;
	DSPint oneChannelByteRate = inputWAVhdr.fmt.ByteRate / inputWAVhdr.fmt.NumChannels;
	DSPint oneChannelBlockAlign = inputWAVhdr.fmt.BlockAlign / inputWAVhdr.fmt.NumChannels;

	outputWAVhdr.data.SubChunk2Size = oneChannelSubChunk2Size * outputWAVhdr.fmt.NumChannels;
	outputWAVhdr.fmt.ByteRate = oneChannelByteRate * outputWAVhdr.fmt.NumChannels;
	outputWAVhdr.fmt.BlockAlign = oneChannelBlockAlign * outputWAVhdr.fmt.NumChannels;


	// Write output WAV header to file
	//-------------------------------------------------
	WriteWavHeader(wav_out, outputWAVhdr);

	// Initialize process

	// Processing loop
	//-------------------------------------------------	
	{
		DSPfract w = 2.0 * M_PI * fc / inputWAVhdr.fmt.SampleRate;
		DSPfract alpha = getAlpha(w);
		getCoefs(alpha, coefs);

		DSPint sample;
		DSPint BytesPerSample = inputWAVhdr.fmt.BitsPerSample / 8;
		const double SAMPLE_SCALE = -(double)(1 << 31);		//2^31
		DSPint iNumSamples = inputWAVhdr.data.SubChunk2Size / (inputWAVhdr.fmt.NumChannels * inputWAVhdr.fmt.BitsPerSample / 8);

		// exact file length should be handled correctly...
		for (DSPint i = 0; i < iNumSamples / BLOCK_SIZE; i++)
		{
			for (DSPint j = 0; j < BLOCK_SIZE; j++)
			{
				for (DSPint k = 0; k < inputWAVhdr.fmt.NumChannels; k++)
				{
					sample = 0; //debug
					fread(&sample, BytesPerSample, 1, wav_in);
					sample = sample << (32 - inputWAVhdr.fmt.BitsPerSample); // force signextend
					sampleBuffer[k][j] = sample / SAMPLE_SCALE;				// scale sample to 1.0/-1.0 range		
				}
			}

			processing();
			

			for (DSPint j = 0; j < BLOCK_SIZE; j++)
			{
				for (DSPint k = 0; k < outputWAVhdr.fmt.NumChannels; k++)
				{
					sample = sampleBuffer[k][j].toLong(); //* SAMPLE_SCALE;//????????????	// crude, non-rounding 			
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