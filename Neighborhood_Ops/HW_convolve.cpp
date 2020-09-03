#include "IP.h"
#include <fstream>

using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_convolve:
//
// Convolve I1 with filter kernel in Ikernel.
// Output is in I2.
//
void
HW_convolve(ImagePtr I1, ImagePtr Ikernel, ImagePtr I2)
{
	std::ofstream outfile;
	outfile.open("test_convolve.txt");
	
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int inputW = I1->width();
	int inputH = I1->height();
	int total = inputW * inputH;

	int kernalW = Ikernel->width();
	int pad_size = kernalW / 2;
	int bufferW = inputW + (kernalW - 1);
	int bufferH = inputH + (kernalW - 1);


	int ch;
	ImagePtr Ibuffer;									//create new image to hold inputimage with pads
	Ibuffer->allocImage(bufferW, bufferH, FFT_TYPE);

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2;
	int type,i,j,p;
	IP_getChannel(Ibuffer, 0, p2, type);
	IP_getChannel(I1, 0, p1, type);

		
	//top padded buffer row(s) with pixel replication
	for (i = 0; i < pad_size; i++) {
		for (j = 0; j < pad_size; j++) *p2++ = *p1;				//top right corner pads
		for (j = 0; j < inputW; j++) *p2++ = *p1++;				//top pad row 
		for (j = 0; j < pad_size; j++) {						//top left corner pads 
			*p2 = *(p2 - 1);
			p2++;
		}
		p1 = I1[0];												//move input pointer back to start
	}
		
	//loop thorugh rows accounting for padding edges
	for ( i = 0; i < inputH; i++) {								//controls row iterations
		for ( j = 0; j < pad_size; j++) *p2++ = *p1;			//number of left pads  
		for ( j = 0; j < inputW; j++) {							//copying input pixels
			*p2++ = *p1++;
		}
		for (j = 0; j < pad_size; j++) {						//number of right pads
			*p2 = *(p2 - 1);
			p2++;
		}
	}
	
	//bottom padded buffer row(s) pixel replication
	for ( i = 0; i < pad_size; i++) {
		p1 = p1 - inputW;										//move to last input row
		for ( j = 0; j < pad_size; j++)*p2++ = *p1;				//bottom right corner pad(s)
		for ( j = 0; j < inputW; j++) *p2++ = *p1++;			//bottom pad row 
		for (j = 0; j < pad_size; j++) {						//bottom left corner pads
			*p2 = *(p2 - 1);
			p2++;
		}
	}

	//Convolve
	ChannelPtr<uchar> p3, p4;
	IP_getChannel(I2, 0, p3, type);
	IP_getChannel(Ikernel, 0, p4, type);
	int sum, k, bposition, kposition;

	
	for (p = 0; p < inputH; p++) {
		for (i = 0; i < inputW; i++) {
			//p2 = &Ibuffer[p];											//move buffer pointer to next pixel	
			sum = 0;													//reset sum
			//p4 = &Ikernel[0];											//move kernel pointer to start

			for (j = 0; j < kernalW; j++) {								//loop thorugh rows
				for (k = 0; k < kernalW; k++) {							//sum the products of current row
					bposition = (p * bufferW + i) + (j * bufferW + k);
					kposition = j * kernalW + k;

					outfile << bposition << '\t' << kposition << '\t' << sum << '\t' << (int)p2[bposition] << '\t' << (int)p4[kposition] << std::endl;
					
					sum += ((int)p4[kposition] * (int)p2[bposition]);
					
				}							
			}
			*p3 = CLIP(sum, 0, 255);										//save convolved value to output image. CLIP for safety
			p3++;														//increment output pointer

		}
	}
}
