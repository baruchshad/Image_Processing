#include "IP.h"
#include <iostream>
using namespace IP;
//using namespace std;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_quantize:
//
// Quantize I1 to specified number of levels. Apply dither if flag is set.
// Output is in I2.
//
void
HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	// init lookup table
	int i, lut[MXGRAY];
	double scale = MXGRAY / levels;							// how many levels to quantize to
	for (i = 0; i < MXGRAY; i++)
		lut[i] = scale * (int)(i / scale) + (scale / 2);	// building the quantization into the LUT.

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2, p3;
	int type;

	// Note: IP_getChannel(I, ch, p1, type) gets pointer p1 of channel ch in image I.
	// The pixel datatype (e.g., uchar, short, ...) of that channel is returned in type.
	// It is ignored here since we assume that our input images consist exclusively of uchars.
	// IP_getChannel() returns 1 when channel ch exists, 0 otherwise.

	if (dither == true) {
		double error;													// contains the dither signal
		bool switched;													// switched alternates the dither signal addition
		// visit all image channels and evaluate output image
		for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {		// get input  pointer for channel ch
			IP_getChannel(I2, ch, p2, type);							// get output pointer for channel ch
			for (int y = 0; y < h; y++) {
				if (y%2 == 0)											// alternate the dither addition from line to line
					switched = true;
				else
					switched = false;									// checks the case where the previous line was even (switched == true)
																		// and the last pixel of the previous row set "switched" to true also.

				for (int x = 0; x < w; x++) {
					error = (rand() / (double)RAND_MAX) * (scale/2);	// generate a random number, normalize to [0,1] and then scale by the bias.
					if (switched == true) {
						error = *p1 + error;							// adding the value of the error with the pixel value.
						switched == false;								// flipping our "switched" to ensure next time the error will be subtracted
					}
					else {
						error = *p1 - error;							// subtracts the value of the dither from the pixel value
						switched == true;								// flipping our "switched" to ensure next time the error will be added
					}
					*p2++ = lut[(int)CLIP(error, 0, 255)];				// use lut[] to eval output and clip any overshoots
					++p1;
				}
			}
		}
	}
	else {															// if the Dither checkbox was not pressed it will just quantize. 
		// visit all image channels and evaluate output image
		for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
			IP_getChannel(I2, ch, p2, type);						// get output pointer for channel ch
			for (i = 0; i < total; i++) {
				*p2++ = lut[*p1++];									// use lut[] to eval output
			}
		}
	}
	
}
