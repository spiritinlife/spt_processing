#include "lodepng.h"
#include <iostream>
#include <time.h>
#include <cmath>


//g++ lodepng.cpp example_decode.cpp -ansi -pedantic -Wall -Wextra -O3

int min(int R, int G, int B){
	if (R < G && R < B)
		return R;
	else if (G < R && G < B)
		return R;
	else
		return B;
}
int max(int R, int G, int B){
	if (R > G && R > B)
		return R;
	else if (G > R && G > B)
		return R;
	else
		return B;
}
struct Pixel {
	int R, G, B, A;
	int GrayLight()
	{
		return (max(R,G,B) + min(R,G,B))/2;
	}
	int GrayLum()
	{
		return 0.21 * R + 0.72 * G + 0.07 * B;
	}
	int GrayAvg()
	{
		return ( R + G + B ) / 3;
	}
};
//Encode from raw pixels to disk with a single function call
//The image argument has width * height RGBA pixels or width * height * 4 bytes
void encodeOneStep(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height)
{
	//Encode the image
	unsigned error = lodepng::encode(filename, image, width, height);

	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}

//Example 3
//Load PNG file from disk using a State, normally needed for more advanced usage.
void decodeWithState(const char* filename)
{
	std::vector<unsigned char> png;
	std::vector<unsigned char> image; //the raw pixels
	unsigned width, height;
	lodepng::State state; //optionally customize this one

	lodepng::load_file(png, filename); //load the image file with given filename
	unsigned error = lodepng::decode(image, width, height, state, png);

	//if there's an error, display it
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	//the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it, ...
	//State state contains extra information about the PNG such as text chunks, ...



	if (width && height) {
		Pixel pixel;
		//generate some image
		std::vector<unsigned char> grayImage(width*height * 4);
		std::vector<unsigned char> tempImage(width*height * 4);

		int size = width*height*4;
		int i = 0;
		int j = 0;
		while (i < size)
		{
			pixel.R = image[i++];
			pixel.G = image[i++];
			pixel.B = image[i++];
			i++; //we dont care aboout A

			float fWB = pixel.GrayLum();
//			float fWB = pixel.GrayAvg();
			//float fWB = pixel.GrayLight();

			grayImage[j++] = fWB;
			grayImage[j++] = fWB;
			grayImage[j++] = fWB;
			grayImage[j++] = 255;

		}




		//we save gray scale image
		encodeOneStep("test_grayscale.png", grayImage, width, height);

		for (int i = 0; i < size; i++)
		{
			tempImage[i] = grayImage[i];
		}
		int maskSize = size - 4 * width;

		//we move 1 pixels at a time on the row, which means 1 * 4 = 4
		//and because R = G = B we only take first value
		for (i = 0; i < maskSize; i += 4)
		{
			unsigned char pixel = tempImage[i];
			unsigned char pixelRow = tempImage[i + 4];
			unsigned char pixelColumn = tempImage[i + 4 * width];

			int Ix = pixel - pixelColumn;
			int Iy = pixel - pixelRow;
			int gradentPixel = sqrt(Ix*Ix + Iy*Iy);

			grayImage[i] = gradentPixel;
			grayImage[i + 1] = gradentPixel;
			grayImage[i + 2] = gradentPixel;

		}



		encodeOneStep("test_edges.png", grayImage, width, height);

	}
}

int main(int argc, char *argv[])
{

	srand(time(NULL));
	const char* filename = argc > 1 ? argv[1] : "test.png";

	decodeWithState(filename);

	std::cout << "Finished.." << std::endl;}
