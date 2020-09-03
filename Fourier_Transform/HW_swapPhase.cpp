#include "IP.h"
#include <cmath>

using namespace IP;

typedef struct {
	int len;																		// length of complex number list
	float * real;																	// pointer to real number list
	float * imag;																	// pointer to imaginary number list
}
complexS, *complexP;

extern void HW_fft2MagPhase(ImagePtr Ifft, ImagePtr Imag, ImagePtr Iphase);
extern void HW_MagPhase2fft(ImagePtr Imag, ImagePtr Iphase, ImagePtr Ifft);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_swapPhase:
//
//Swap the phase channels of I1 and I2.
// Output is in II1 and II2.
//
// Swap phase of I1 with I2.
// (I1_mag, I2_phase) -> II1
// (I1_phase, I2_mag) -> II2
//
void
HW_swapPhase(ImagePtr I1, ImagePtr I2, ImagePtr II1, ImagePtr II2)
{
	ImagePtr Ifft1, Ifft2, IinvFFT1, IinvFFT2;
	ImagePtr Imag1, Iphase1, Imag2, Iphase2;

	ChannelPtr<uchar> in;
	ChannelPtr<float> real, imag;
	int i, j, type,position, w, h, wpad, hpad, dir, total;
	float *inputreal, *inputimg, *outputreal, *outputimg;
	complexS q1, q2;
	complexP p1, p2;

	void fft1D(complexP, int, complexP);

	w = I1->width();
	h = I1->height();
	//total = w * h;

	wpad = (w % 2) + w;																// total width with power of 2 padding
	hpad = (h % 2) + h;																// total height with power of 2 padding
	total = hpad * wpad;

	IP_copyImageHeader(I1, II1);													// copy image header (width, height) of input image I1 to output image II2
	IP_copyImageHeader(I2, II2);													// copy image header (width, height) of input image I1 to output image II2
	Ifft1->allocImage(wpad, hpad, FFT_TYPE);
	Ifft2->allocImage(wpad, hpad, FFT_TYPE);

	inputreal = new float[wpad];
	inputimg = new float[wpad];
	outputreal = new float[wpad];
	outputimg = new float[wpad];

// compute FFT of I1 and I2

	dir = 0;																		// flag set to Forward FFT.

	// I1 FFT
	IP_getChannel(I1, 0, in, type);
	IP_getChannel(Ifft1, 0, real, type);											// channel 0 hold real coeff.
	IP_getChannel(Ifft1, 1, imag, type);											// channel 1 holds imaginary coeff.
	
	// I1 -- 1D pass on rows
	for (i = 0; i < hpad; i++) {
		for(j=0;j < wpad; j++) {					// Pass row elements into vectors
			position = i * w + j;
			inputreal[j] = in[position];
			inputimg[j] = 0;
		}

		for (; j < wpad; j++) {                             //pads input array with extra zeros
			real[j] = 0;
			imag[j] = 0;
		}

		//for (i = 0; i < n; i++) cout << inputreal[i] << endl; //testing
		//cout << "\n\n\n" << endl;								//testing

		//set all data points
		q1.len = wpad;
		q2.len = wpad;
		q1.real = inputreal;
		q2.real = outputreal;
		q1.imag = inputimg;
		q2.imag = outputimg;

		//pointer to complex number struct
		p1 = &q1;
		p2 = &q2;
		fft1D(p1, dir, p2);

		//save 1D pass.
		for (j = 0; j < wpad; j++) {
			position = i * w + j;
			real[position] = outputreal[j];
			imag[position] = outputimg[j];
		}
	}
	
	//I1 -- 2D pass on columns
	for (i = 0; i < wpad; i++) {

		for (j = 0; j < hpad; j++) {						// Pass column elements into vectors
			position = j * w + i;
			inputreal[j] = real[position];
			inputimg[j] = imag[position];
		}

		for (; j < hpad; j++) {                             //pads input array with extra zeros
			inputreal[j] = 0;
			inputimg[j] = 0;
		}

		//set all data points
		q1.len = hpad;
		q2.len = hpad;
		q1.real = inputreal;
		q2.real = outputreal;
		q1.imag = inputimg;
		q2.imag = outputimg;

		//pointer to complex number struct
		p1 = &q1;
		p2 = &q2;
		fft1D(p1, dir, p2);

		//save 2D pass.
		for (j = 0; j < hpad; j++) {
			position = j * w + i;
			real[position] = outputreal[j];
			imag[position] = outputimg[j];
		}
	}


	// I2 FFT
	IP_getChannel(I2, 0, in, type);	
	IP_getChannel(Ifft2, 0, real, type);											// channel 0 hold real coeff.
	IP_getChannel(Ifft2, 1, imag, type);											// channel 1 holds imaginary coeff.

	// I2  --  1D pass on rows
	for (i = 0; i < h; i++) {

		for (j = 0; j < w; j++) {					// Pass row elements into vectors
			position = i * w + j;
			inputreal[j] = in[position];
			inputimg[j] = 0;
		}

		for (; j < wpad; j++) {                             //pads input array with extra zeros
			inputreal[j] = 0;
			inputimg[j] = 0;
		}

		//set all data points
		q1.len = wpad;
		q2.len = wpad;
		q1.real = inputreal;
		q2.real = outputreal;
		q1.imag = inputimg;
		q2.imag = outputimg;

		//pointer to complex number struct
		p1 = &q1;
		p2 = &q2;
		fft1D(p1, dir, p2);

		//save 1D pass.
		for (j = 0; j < wpad; j++) {
			position = i * w + j;
			real[position] = outputreal[j];
			imag[position] = outputimg[j];
		}
	}

	//I2 -- 2D pass on columns
	for (i = 0; i < wpad; i++) {

		for (j = 0; j < h; j++) {					// Pass column elements into vectors
			position = j * w + i;
			inputreal[j] = real[position];
			inputimg[j] = imag[position];
		}

		for (; j < hpad; j++) {                             //pads input array with extra zeros
			inputreal[j] = 0;
			inputimg[j] = 0;
		}

		//set all data points
		q1.len = hpad;
		q2.len = hpad;
		q1.real = inputreal;
		q2.real = outputreal;
		q1.imag = inputimg;
		q2.imag = outputimg;

		//pointer to complex number struct
		p1 = &q1;
		p2 = &q2;
		fft1D(p1, dir, p2);

		//save 2D pass.
		for (j = 0; j < hpad; j++) {
			position = j * w + i;
			real[position] = outputreal[j];
			imag[position] = outputimg[j];
		}
	}


// compute magnitude and phase from real and imaginary FFT channels
	
	ChannelPtr<float> mag, phase;

	Imag1->allocImage(wpad, hpad, FFT_TYPE);										// allocating space. 
	Iphase1->allocImage(wpad, hpad, FFT_TYPE);
	Imag2->allocImage(wpad, hpad, FFT_TYPE);							 
	Iphase2->allocImage(wpad, hpad, FFT_TYPE);
	
	// I1 magnitude and phase calculations
	IP_getChannel(Ifft1, 0, real, type);											// channel 0 hold real coeff.
	IP_getChannel(Ifft1, 1, imag, type);											// channel 1 holds imaginary coeff.
	IP_getChannel(Imag1, 0, mag, type);
	IP_getChannel(Iphase1, 0, phase, type);

	for (i = 0; i < total; i++) {
		mag[i] = sqrt(pow(real[i],2) + pow(imag[i], 2));
		phase[i] = atan2(imag[i], real[i]);											// for atan2 function is (x,y)

	}


	// I2 magnitude and phase calculations
	IP_getChannel(Ifft2, 0, real, type);											// channel 0 hold real coeff.
	IP_getChannel(Ifft2, 1, imag, type);											// channel 1 holds imaginary coeff.
	IP_getChannel(Imag2, 0, mag, type);
	IP_getChannel(Iphase2, 0, phase, type);

	for (i = 0; i < total; i++) {
		mag[i] = sqrt(pow(real[i], 2) + pow(imag[i], 2));
		phase[i] = atan2(imag[i], real[i]);										// for atan2 function is (x,y)
	}


// swap phases and convert back to FFT images

	// I1 coefficents with swap
	IP_getChannel(Ifft1, 0, real, type);											// channel 0 hold real coeff.
	IP_getChannel(Ifft1, 1, imag, type);											// channel 1 holds imaginary coeff.
	IP_getChannel(Imag1, 0, mag, type);												// Magnitude of image I1
	IP_getChannel(Iphase2, 0, phase, type);											// Phase of I2

	for (i = 0; i < total; i++) {
		real[i] = cos(phase[i]) * mag[i];
		imag[i] = sin(phase[i]) * mag[i];
	}

	// I2 coefficents with swap
	IP_getChannel(Ifft2, 0, real, type);											// channel 0 hold real coeff.
	IP_getChannel(Ifft2, 1, imag, type);											// channel 1 holds imaginary coeff.
	IP_getChannel(Imag2, 0, mag, type);												// Magnitude of I2
	IP_getChannel(Iphase1, 0, phase, type);											// Phase of I1

	for (i = 0; i < total; i++) {
			real[i] = cos(phase[i]) * mag[i];
			imag[i] = sin(phase[i]) * mag[i];
	}


// compute inverse FFT
	
	dir = 1;																		// flag set to Inverse FFT

// I1 FFT
	IP_getChannel(Ifft1, 0, real, type);											// channel 0 hold real coeff.
	IP_getChannel(Ifft1, 1, imag, type);											// channel 1 holds imaginary coeff.

	// I1 -- 1D pass on rows
	for (i = 0; i < hpad; i++) {
		for (j = 0; j < wpad; j++) {												// Pass row elements into vectors
			position = i * w + j;
			inputreal[j] = real[position];
			inputimg[j] = imag[position];
		}

		//set all data points
		q1.len = wpad;
		q2.len = wpad;
		q1.real = inputreal;
		q2.real = outputreal;
		q1.imag = inputimg;
		q2.imag = outputimg;

		//pointer to complex number struct
		p1 = &q1;
		p2 = &q2;
		fft1D(p1, dir, p2);

		//save 1D pass.
		for (j = 0; j < wpad; j++) {
			position = i * w + j;
			real[position] = outputreal[j];
			imag[position] = outputimg[j];
		}
	}

	//I1 -- 2D pass on columns
	for (i = 0; i < wpad; i++) {

		for (j = 0; j < hpad; j++) {						// Pass column elements into vectors
			position = j * w + i;
			inputreal[j] = real[position];
			inputimg[j] = imag[position];
		}

		//set all data points
		q1.len = hpad;
		q2.len = hpad;
		q1.real = inputreal;
		q2.real = outputreal;
		q1.imag = inputimg;
		q2.imag = outputimg;

		//pointer to complex number struct
		p1 = &q1;
		p2 = &q2;
		fft1D(p1, dir, p2);

		//save 2D pass.
		for (j = 0; j < hpad; j++) {
			position = j * w + i;
			real[position] = outputreal[j];
			imag[position] = outputimg[j];
		}
	}

// I2 FFT
	IP_getChannel(Ifft2, 0, real, type);											// channel 0 hold real coeff.
	IP_getChannel(Ifft2, 1, imag, type);											// channel 1 holds imaginary coeff.

	// I2  --  1D pass on rows
	for (i = 0; i < h; i++) {

		for (j = 0; j < w; j++) {					// Pass row elements into vectors
			position = i * w + j;
			inputreal[j] = real[position];
			inputimg[j] = imag[position];
		}

		//set all data points
		q1.len = wpad;
		q2.len = wpad;
		q1.real = inputreal;
		q2.real = outputreal;
		q1.imag = inputimg;
		q2.imag = outputimg;

		//pointer to complex number struct
		p1 = &q1;
		p2 = &q2;
		fft1D(p1, dir, p2);

		//save 1D pass.
		for (j = 0; j < wpad; j++) {
			position = i * w + j;
			real[position] = outputreal[j];
			imag[position] = outputimg[j];
		}
	}

	//I2 -- 2D pass on columns
	for (i = 0; i < wpad; i++) {

		for (j = 0; j < h; j++) {					// Pass column elements into vectors
			position = j * w + i;
			inputreal[j] = real[position];
			inputimg[j] = imag[position];
		}

		//set all data points
		q1.len = hpad;
		q2.len = hpad;
		q1.real = inputreal;
		q2.real = outputreal;
		q1.imag = inputimg;
		q2.imag = outputimg;

		//pointer to complex number struct
		p1 = &q1;
		p2 = &q2;
		fft1D(p1, dir, p2);

		//save 2D pass.
		for (j = 0; j < hpad; j++) {
			position = j * w + i;
			real[position] = outputreal[j];
			imag[position] = outputimg[j];
		}
	}



// extract magnitude from resulting images

	// I1 magnitude 
	IP_getChannel(Ifft1, 0, real, type);											// channel 0 hold real coeff.
	IP_getChannel(Ifft1, 1, imag, type);											// channel 1 holds imaginary coeff.
	IP_getChannel(Imag1, 0, mag, type);												// magnitude of image I1

	for (i = 0; i < total; i++) {
		mag[i] = sqrt(pow(real[i], 2) + pow(imag[i], 2));
	}

	// I2 magnitude
	IP_getChannel(Ifft2, 0, real, type);											// channel 0 hold real coeff.
	IP_getChannel(Ifft2, 1, imag, type);											// channel 1 holds imaginary coeff.
	IP_getChannel(Imag2, 0, mag, type);

	for (i = 0; i < total; i++) {
		mag[i] = sqrt(pow(real[i], 2) + pow(imag[i], 2));
	}


// allocate uchar image and cast float channel to uchar for mag1

	IP_getChannel(II1, 0, in, type);
	IP_getChannel(Imag1, 0, mag, type);												// magnitude of image I1

	for (i = 0; i < total; i++) {
			in[i] = CLIP(mag[i],0,MaxGray);											// CLIP prevents over and undershoots

	}

	IP_getChannel(II2, 0, in, type);
	IP_getChannel(Imag2, 0, mag, type);												// magnitude of image I1
	for (i = 0; i < total; i++) {
			in[i] = CLIP(mag[i], 0, MaxGray);										// CLIP prevents over and undershoots
	}

}

/******************************************************************************
Name: fft1D
Description: fast fourier transform in 1D
Calls: None
Called by: main
*******************************************************************************/
void fft1D(complexP q1, int dir, complexP q2)
{
	int i, N, N2;
	float *r1, *i1, *r2, *i2, *ra, *ia, *rb, *ib;
	float FCTR, fctr, a, b, c, s, num[2];
	complexP qa, qb;

	N = q1->len;
	r1 = (float *)q1->real;
	i1 = (float *)q1->imag;
	r2 = (float *)q2->real;
	i2 = (float *)q2->imag;

	if (N == 2) {
		a = r1[0] + r1[1]; // F(0)=f(0)+f(1);F(1)=f(0)-f(1)
		b = i1[0] + i1[1]; // a,b needed when r1=r2
		r2[1] = r1[0] - r1[1];
		i2[1] = i1[0] - i1[1];
		r2[0] = a;
		i2[0] = b;

	}
	else {
		N2 = N / 2;
		qa = (complexP)malloc(sizeof(complexS));
		qa->len = N2;
		qa->real = (float *)malloc(sizeof(float) * qa->len);
		qa->imag = (float *)malloc(sizeof(float) * qa->len);

		qb = (complexP)malloc(sizeof(complexS));
		qb->len = N2;
		qb->real = (float *)malloc(sizeof(float) * qb->len);
		qb->imag = (float *)malloc(sizeof(float) * qb->len);

		ra = (float *)qa->real;
		ia = (float *)qa->imag;
		rb = (float *)qb->real;
		ib = (float *)qb->imag;

		// split list into 2 halves; even and odd
		for (i = 0; i < N2; i++) {
			ra[i] = *r1++;
			ia[i] = *i1++;
			rb[i] = *r1++;
			ib[i] = *i1++;
		}

		// compute fft on both lists
		fft1D(qa, dir, qa);
		fft1D(qb, dir, qb);

		// build up coefficients
		if (!dir) // forward
			FCTR = -2 * PI / N;
		else FCTR = 2 * PI / N;

		for (fctr = i = 0; i < N2; i++, fctr += FCTR) {
			c = cos(fctr);
			s = sin(fctr);
			a = c * rb[i] - s * ib[i];
			r2[i] = ra[i] + a;
			r2[i + N2] = ra[i] - a;
			a = s * rb[i] + c * ib[i];
			i2[i] = ia[i] + a;
			i2[i + N2] = ia[i] - a;
		}
		free(qa);
		free(qb);
	}

	if (!dir) { // inverse : divide by logN
		for (i = 0; i < N; i++) {
			q2->real[i] = q2->real[i] / 2;
			q2->imag[i] = q2->imag[i] / 2;
		}
	}

}

