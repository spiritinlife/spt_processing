
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
//opencv
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/video/video.hpp>
#include <stdio.h>
#include <cv.h>

using namespace cv;
using namespace std;

Mat prevFrame;

//hide the local functions in an anon namespace
namespace {
    void help(char** av) {
        cout << "The program captures frames from a video file, image sequence (01.jpg, 02.jpg ... 10.jpg) or camera connected to your computer." << endl
                << "Usage:\n" << av[0] << " <video file, image sequence or device number>" << endl
                << "q,Q,esc -- quit" << endl
                << "space   -- save frame" << endl << endl
                << "\tTo capture from a camera pass the device number. To find the device number, try ls /dev/video*" << endl
                << "\texample: " << av[0] << " 0" << endl
                << "\tYou may also pass a video file instead of a device number" << endl
                << "\texample: " << av[0] << " video.avi" << endl
                << "\tYou can also pass the path to an image sequence and OpenCV will treat the sequence just like a video." << endl
                << "\texample: " << av[0] << " right%%02d.jpg" << endl;
    }


    int process(VideoCapture& capture) {
        int n = 0;

        char filename[200];
        string window_name = "video | q or esc to quit";
        cout << "press space to save a picture. q or esc to quit" << endl;
        namedWindow(window_name, WINDOW_KEEPRATIO); //resizable window;
        Mat frame;


        Point *pul,*pbr;
        pul = NULL;
        pbr = NULL;

        for (;;) {
            capture >> frame;
            if (frame.empty())
                break;
            if (prevFrame.empty())
                prevFrame = frame.clone();
            Mat nframe = frame.clone();



            //cvtColor(frame, nframe, CV_BGR2GRAY);

            int channels = nframe.channels();
            int nRows = nframe.rows;
            int nCols = nframe.cols * channels;

            if (nframe.isContinuous())
            {
                nCols *= nRows;
                nRows = 1;
            }
          /* int width = capture.get(3);
            uchar *row = NULL;
            double Ix = 0;
            double Iy = 0;
            for (int i = 0; i < nRows; ++i) {
                row = nframe.ptr<uchar>(i);
                for (int j = 0; j < nCols; j++)
                {

                    Ix = row[j] - row[j+channels];
                    Iy = row[j] - row[j+width+channels];

                    nframe.ptr<uchar>(i)[j] = sqrt(Ix*Ix + Iy*Iy);
                }

            }*/

            int i,j;
            uchar* np;
            uchar* op;
            Point *ul,*br;
            ul = NULL;
            br = NULL;
            int width = capture.get(3);
            int height = capture.get(4);
            for( i = 0; i < nRows; ++i)
            {
                np = frame.ptr<uchar>(i);
                op = prevFrame.ptr<uchar>(i);
                int check1 = 5;
                int check2 = 5;
                for ( j = 0; j < nCols; ++j)
                {
                      for (int k = j; k < j + channels ; ++k) {
                          uchar  dif =  abs(op[k] - np[k]);
                          if (dif > 10) {
                              for (int l = 0; l < channels; ++l) {
                                  uchar  diff =  abs(op[j + l] - np[j + l]);

                                  nframe.ptr<uchar>(i)[j + l] = diff;
                              }
                          }
                          else if (k == j + channels -1)
                          {
                              for (int l = 0; l < channels; ++l) {
                                  nframe.ptr<uchar>(i)[j + l] = 0;
                              }
                          }
                          if (dif > 130  && check1 <= 5){
                              ul = new Point((j/channels)%width,(j/channels)/width);
                              check1--;
                          }
                          else if (dif > 60 && ul!= NULL && check2 <= 5) {
                              br = new Point((j/channels)%width,(j/channels)/width);
                              check2 --;
                          }
                          else {
                              check1 = 5;
                              check2 = 5;
                          }
                          if (check1 <= 0)
                              check1 = 6;

                          if (check2 <= 0)
                              check2 = 6;
                      }


                }
            }

        if (pul != NULL && pbr != NULL)
            rectangle(nframe, *pul, *pbr,
                    cv::Scalar(140, 89, 255), 11,6);
            if (ul != NULL && br != NULL) {
                cout << *ul << " " << *br <<  endl;
                pul = ul;
                pbr = br;
                ul = NULL;
                br = NULL;
            }
            imshow(window_name, nframe);
            prevFrame = frame.clone();
            char key = (char) waitKey(30); //delay N millis, usually long enough to display and capture input

            switch (key) {
                case 'q':
                case 'Q':
                case 27: //escape key
                    return 0;
                case ' ': //Save an image
                    sprintf(filename, "filename%.3d.jpg", n++);
                    imwrite(filename, frame);
                    cout << "Saved " << filename << endl;
                    break;
                default:
                    break;

            }

        }
        return 0;
    }
}



Mat frame; //current frame
Mat fgMaskMOG2; //fg mask fg mask generated by MOG2 method
int keyboard; //input from keyboard

int processVideo(VideoCapture& capture) {
    if(!capture.isOpened()){
        //error in opening the video input
        exit(EXIT_FAILURE);
    }
    Ptr<BackgroundSubtractor> pMOG2 = createBackgroundSubtractorMOG2(); //MOG2 approach
    //read input data. ESC or 'q' for quitting
    while( (char)keyboard != 'q' && (char)keyboard != 27 ){
        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
        //update the background model
        pMOG2->apply(frame, fgMaskMOG2,-2);
        //get the frame number and write it on the current frame
        stringstream ss;
        rectangle(frame, cv::Point(10, 2), cv::Point(100,20),
                cv::Scalar(140,89,255), -1);
        ss << capture.get(CAP_PROP_POS_FRAMES);
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks
        imshow("Frame", frame);
        imshow("FG Mask MOG 2", fgMaskMOG2);
        //get the input from the keyboard
        keyboard = waitKey( 30 );
    }
    //delete capture object
    capture.release();
    return 0;
}
int main(int ac, char** av) {


    std::string arg = "/dev/video0";
    VideoCapture capture(arg); //try to open string, this will attempt to open it as a video file or image sequence
    if (!capture.isOpened()) //if this fails, try to open as a video camera, through the use of an integer param
        capture.open(atoi(arg.c_str()));
    if (!capture.isOpened()) {
        cerr << "Failed to open the video device, video file or image sequence!\n" << endl;
        help(av);
        return 1;
    }
    return process(capture);
    //return processVideo(capture);
}
