#include "IP.h"
using namespace IP;

void histoMatchApprox(ImagePtr, ImagePtr, ImagePtr);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoMatch:
//
// Apply histogram matching to I1. Output is in I2.
//
void
HW_histoMatch(ImagePtr I1, ImagePtr targetHisto, bool approxAlg, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	if (approxAlg) { //the default code given.
		histoMatchApprox(I1, targetHisto, I2);
		return;
	}

	int i, p, R, type, total, Hsum, Havg, *h2;
	int left[MXGRAY], right[MXGRAY], reserve[MXGRAY], h1[MXGRAY];
	ChannelPtr<uchar> in, out, lutp;
	double scale;

	// total number of pixels in image
	total = (long)I1->height() * I1->width();


	for (i = 0; i < MXGRAY; i++) h1[i] = 0;			// clear histogram
	in = I1[0];										// get first channel
	for (i = 0; i < total; i++) h1[in[i]]++;		// eval histogram

	// target histogram
	lutp = targetHisto[0];    // get first channel
	h2 = (int *)&lutp[0];

	// normalize h2 to conform with dimensions of I1
	for (i = Havg = 0; i < MXGRAY; i++) Havg += h2[i];
	scale = (double)total / Havg;
	if (scale != 1) for (i = 0; i < MXGRAY; i++) h2[i] *= scale;

	R = 0;
	Hsum = 0;
	// evaluate remapping of all input gray levels;
	// Each input gray value maps to an interval of valid
	// output values. The endpoints of the intervals are
	// left[] and right[] 
	for (i = 0; i < MXGRAY; i++) {
		left[i] = R;								// left end of interval
		Hsum += h1[i];								// cum. interval value
		while (Hsum > h2[R] && R < MXGRAY - 1) {	// make interval wider, if necessary
			Hsum -= h2[R];							// adjust Hsum
			R++;									// move to next bin
		}
		reserve[i] = Hsum;							// reserve the spill over to the last bin for future use
		right[i] = R;								// init right end of interval
	}

	
	// visit all input pixels
	for (i = 0; i < MXGRAY; i++) h1[i] = 0;										// clear h1 for reuse
	R = 0;																		// clear R for reuse
	out = I2[0];
	for (i = 0; i < total; i++) {
		p = left[in[i]];
		R = in[i];
		while (p == right[R]) reserve[R++]--;									//remove reserved spaces for bin 
		if ((in[i] == 0 || left[in[i]] != right[in[i] - 1]) && h1[p] < h2[p]){
			out[i] = p;															// not part of reserved space
		}
		else {
			if ((h1[p] + reserve[in[i] - 1]) < h2[p])							//use this instead of if below
				out[i] = p;
			else out[i] = p = left[in[i]] = MIN(p + 1, right[in[i]]);
		}
		h1[p]++;
	}

}

void
histoMatchApprox(ImagePtr I1, ImagePtr targetHisto, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	int i, p, R, type, left[MXGRAY], right[MXGRAY];
	int total, Hsum, Havg, h1[MXGRAY], *h2;
	ChannelPtr<uchar> in, out, lutp;
	double scale;

	// total number of pixels in image
	total = (long)I1->height() * I1->width();

	for (i = 0; i < MXGRAY; i++) h1[i] = 0;			// clear histogram

	IP_getChannel(I1, 0, in, type);
	for (i = 0; i < total; i++) h1[in[i]]++;		// eval histogram

	// target histogram
	lutp = targetHisto[0];							// get first channel
	h2 = (int *)&lutp[0];

	// normalize h2 to conform with dimensions of I1
	for (i = Havg = 0; i < MXGRAY; i++) Havg += h2[i];
	scale = (double)total / Havg;
	if (scale != 1) for (i = 0; i < MXGRAY; i++) h2[i] *= scale;

	R = 0;
	Hsum = 0;
	// evaluate remapping of all input gray levels;
	// Each input gray value maps to an interval of valid
	// output values. The endpoints of the intervals are
	// left[] and right[] 
	for (i = 0; i < MXGRAY; i++) {
		left[i] = R;								// left end of interval
		Hsum += h1[i];								// cum. interval value
		while (Hsum > h2[R] && R < MXGRAY - 1) {	// make interval wider, if necessary
			Hsum -= h2[R];							// adjust Hsum
			R++;									// move to next bin
		}
		right[i] = R;								// init right end of interval
	}

	for (i = 0; i < MXGRAY; i++) h1[i] = 0;			// clear h1 and reuse it below
	
	IP_getChannel(I2, 0, out, type);
	for (i = 0; i < total; i++) {					// visit all input pixels and map to output
		p = left[in[i]];
		if (h1[p] < h2[p])							// mapping satisfies h2
			out[i] = p;
		else out[i] = p = left[in[i]] = MIN(p + 1, right[in[i]]);
		h1[p]++;
	}

}
