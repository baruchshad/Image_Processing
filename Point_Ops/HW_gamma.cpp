#include "IP.h"
#include <cmath>
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_gammaCorrect:
//
// Gamma correct image I1. Output is in I2.
//
void
HW_gammaCorrect(ImagePtr I1, double gamma, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	int i, lut[MXGRAY];
	double x, hold;
	for (i = 0; i < MaxGray; i++){
		x = (double)i / 255;					// normalized to 0,255
		hold = 255 * pow(x, 1 / gamma);			// gamma function to adjust pixels	
		lut[i] = (int)CLIP(hold, 0, 255);		// CLIP just in case for bad rounding errors
	}

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type;

	// Note: IP_getChannel(I, ch, p1, type) gets pointer p1 of channel ch in image I.
	// The pixel datatype (e.g., uchar, short, ...) of that channel is returned in type.
	// It is ignored here since we assume that our input images consist exclusively of uchars.
	// IP_getChannel() returns 1 when channel ch exists, 0 otherwise.

	// visit all image channels and evaluate output image

	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++){
		IP_getChannel(I2, ch, p2, type);
		for (i = 0; i < total; i++)
			*p2++ = lut[*p1++];
	}
}
