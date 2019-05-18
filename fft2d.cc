// Distributed two-dimensional Discrete FFT transform
// YOUR NAME HERE

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <complex>
#include <thread>
#include <cstdlib>

#include <atomic>

#include "Complex.h"
#include "InputImage.h"
#include <chrono>

namespace sc = std::chrono;

constexpr unsigned int NUMTHREADS = 4;

using namespace std;

//We will assume NUMTHREADS will evenly divide the number of rows in tested images
// I will test with a different image than the one given

void Transpose(Complex* in, int w, int h, Complex* out){
        int ind = 0;
    for (int i = 0; i < h; i++){
        for (int j = 0; j < w; j++){
            out[ind] = in[i+j*w];
                    ind++;
        }
    }
}

void Transform1D(Complex* h, int w, Complex* H)
{
  // Implement a simple 1-d DFT using the double summation equation
  // given in the assignment handout.  h is the time-domain input
  // data, w is the width (N), and H is the output array.

    //Complex W(0,0);
    for (int i = 0; i < w; i++){
        Complex sum(0,0);
    for (int k = 0; k < w; k++){
        //Complex W( cos(-2*M_PI*i*k/w) / w, -sin(-2*M_PI*i*k/w)/w);
        Complex W(cos(2*M_PI*i*k/w), -sin(2*M_PI*i*k/w));
        sum = sum + W*h[k];
    }
        H[i] = sum;
    }
}

void invTransform1D(Complex* h, int w, Complex* H)
{
    // Implement a simple 1-d iDFT using the double summation equation
    // given in the assignment handout.  h is the time-domain out
    // data, w is the width (N), and H is the frequency-domain input array.

    //Complex W(0,0);
    for (int i = 0; i < w; i++){
        Complex sum(0,0);
        for (int k = 0; k < w; k++){
            Complex W( cos(-2*M_PI*i*k/w) / w, -sin(-2*M_PI*i*k/w)/w);
            sum = sum + W*H[k];
        }
        h[i] = sum;
    }
}

void forwardThread(Complex* h, int w, Complex* H,int thread, int rows)
{
    int ind = thread*rows*w;
    for (int j = 0; j < rows; j++){
        //Transform1D(h + j*w*rows + w*thread,w,H + j*w*rows + w*thread);
        Transform1D(h + j*w + ind,w,H + j*w + ind);
    }
}

void invThread(Complex* h, int w, Complex* H,int thread, int rows)
{
    int ind = thread*rows*w;
    for (int j = 0; j < rows; j++){
        //Transform1D(h + j*w*rows + w*thread,w,H + j*w*rows + w*thread);
        invTransform1D(h + j*w + ind,w,H + j*w + ind);
    }
}



void Transform2D(const char* inputFN)
{ // Do the 2D transform here.
    // 1) Use the InputImage object to read in the Tower.txt file and
    //    find the width/height of the input image.
    InputImage image(inputFN);
    int width = image.GetWidth();
    int height = image.GetHeight();// Create the helper object for reading the image

    // 2) Create a vector of complex objects of size width * height to hold
    //    values calculated
    Complex* data = image.GetImageData();
    int arrlen = width*height;
    Complex* fvalues = new Complex[arrlen]();
    Complex* fvaluest = new Complex[arrlen]();
    Complex* fvalues2dt = new Complex[arrlen]();
    Complex* fvalues2d = new Complex[arrlen]();


    // 3) Do the individual 1D transforms on the rows assigned to each thread
    // SINGLETHREAD ~~
//    for (int j = 0; j < height; j++){
//        Transform1D(data + j*width,width,fvalues + j*width);
//    }

    //Multithreading !~~~ //
    int rowsperThread = height/NUMTHREADS;
    std::thread threads[NUMTHREADS];

    for (int j = 0; j < NUMTHREADS; j++){
        threads[j] = std::thread(forwardThread,data,width,fvalues, j, rowsperThread);
    }


    // 4) Force each thread to wait until all threads have completed their row calculations
    //    prior to starting column calculations
    for (int m = 0; m < NUMTHREADS; m++ ) {
        threads[m].join();
    }

    // 5) Perform column calculations
    Transpose(fvalues,width,height,fvaluest);

    // SINGLETHREAD ~~
//    for (int j = 0; j < height; j++){
//        Transform1D(fvaluest + j*width,width,fvalues2dt + j*width);
//    }

    //Multithreading !~~~ //
    int colsperThread = width/NUMTHREADS;
    for (int z = 0; z < NUMTHREADS; z++){
        threads[z] = std::thread(forwardThread,fvaluest,height,fvalues2dt, z, colsperThread);
    }

    // 6) Wait for all column calculations to complete
    for (int m = 0; m < NUMTHREADS; m++ ) {
        threads[m].join();
    }
//
//    // 7) Use SaveImageData() to output the final results
    Transpose(fvalues2dt,width,height,fvalues2d);

    image.SaveImageData("MyAfter2D.txt",fvalues2d,width,height);
//
    // Now lets do it inverse !

    //initialize some empty vectors...
    Complex* tvalues1d = new Complex[arrlen]();
    Complex* tvalues1dt = new Complex[arrlen]();
    Complex* tvalues2dt = new Complex[arrlen]();
    Complex* tvalues2d = new Complex[arrlen]();

    // do the inverse transform on the rows

    for (int j = 0; j < NUMTHREADS; j++){
        threads[j] = std::thread(invThread,tvalues1d,width,fvalues2d, j, rowsperThread);
    }

    // wait for all threads to complete

    for (int m = 0; m < NUMTHREADS; m++ ) {
        threads[m].join();
    }

    // transpose then do inv transform on all the 'cols'
    Transpose(tvalues1d,width,height,tvalues1dt);

    for (int z = 0; z < NUMTHREADS; z++){
        threads[z] = std::thread(invThread,tvalues2dt,height,tvalues1dt, z, colsperThread);
    }

    // 6) Wait for all column calculations to complete
    for (int m = 0; m < NUMTHREADS; m++ ) {
        threads[m].join();
    }

    // transpose one more time to get original file
    Transpose(tvalues2dt,width,height,tvalues2d);
    image.SaveImageDataReal("MyAfterInverse.txt",tvalues2d,width,height);

    delete tvalues1d;
    delete tvalues1dt;
    delete tvalues2d;
    delete tvalues2dt;
    delete fvalues;
    delete fvalues2d;
    delete fvalues2dt;
    delete fvaluest;

}

int main(int argc, char** argv)
{
  string fn("Tower.txt"); // default file name
  if (argc > 1) fn = string(argv[1]);  // if name specified on cmd line
    auto start = sc::high_resolution_clock::now();
  Transform2D(fn.c_str()); // Perform the transform.
    auto end = sc::high_resolution_clock::now();
    std::cout << "Elapsed Time: " << sc::duration_cast<sc::milliseconds>(end - start).count() << "ms" << std::endl;
}  
  

  
