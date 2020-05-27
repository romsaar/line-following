#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;
/*******************
 * PROTOTYPES
 * *****************/
int find_line(Mat& frame);
void display_image(Mat gray);

int get_strong_lines(vector<Vec2f>& lines, vector<Vec2f>& strong_lines, float rho_min_dst);
/**********************
 * GLOBALS
 * ********************/
bool Probabilistic = false;
void myprintf(const char * fmt, const char* fn, ...)
{
    char chbuf[1024];
    #if 1
    va_list args;
    //va_start(args, chfmt);
    strncpy(chbuf, fn, 1024);  
    strcat(chbuf, ": ");  
    va_start(args, fn);

    //vsnprintf(chbuf, sizeof(chbuf), chfmt, args);
    vsnprintf(chbuf+strlen(chbuf), sizeof(chbuf)-strlen(chbuf), fmt, args);
    va_end(args); 
    #endif
    fprintf(stdout, "%s", chbuf);
}

/******************
 * Find line
 * ***************/
int find_line(Mat& frame)
{
    const char * fn="find_line()";
    /*
     A function used to find distinct lines in an images stream

    Attributes:
    ----------
    cap -
    rho_min_dist - the min dist between distinct lines

    Output:
    ------
    strong_lines - a structure including up to 4 distinct lines
    
     */ 
    int N;
    int height,width;
    cv::Size size;
    cv::Size s = frame.size();
    height = s.height;
    width = s.width;
    myprintf("height:%d width%d\n", fn, height, width);
    //int Sz[] = {height, width, 3};
    cv::Mat normalizedFrame = cv::Mat(height, width, CV_8UC3, cv::Scalar(0,0,0));
    cv::Mat gray = cv::Mat(height, width, CV_8UC1, cv::Scalar(0));
    myprintf("Init gray channels:%d\n", fn, gray.channels()); // 1 channel
    cv::normalize(frame, normalizedFrame, 0, 255, cv::NORM_MINMAX);
    //Translate from CV2 BGR to gray scale
    cvtColor(normalizedFrame, gray, cv::COLOR_BGR2GRAY);
    myprintf("normalizedFrame channels:%d\n", fn, normalizedFrame.channels()); //3 channels
    myprintf("gray channels:%d size:%dx%d\n", fn, gray.channels(), 
                    gray.size().height, gray.size().width); // 1 channel
    //Canny Edge Detection:
    float C_Threshold1 = 20; // 200 // pixels with lower gradient are declared not-edge
    float C_Threshold2 = 50; // 500 // pixels with higher gradient are declared edge
    //edges = cv2.Canny(gray, C_Threshold1, C_Threshold2, apertureSize=3)
    cv::Mat edges = cv::Mat(height, width, CV_8UC1, cv::Scalar(0));
    Canny(gray, edges, C_Threshold1, C_Threshold2, 3);
  
    display_image(edges);
    // Hough Transform
    if (Probabilistic)
    {
        vector<Vec4i> lines;
        double rho=1.0;
        double theta=1 * CV_PI / 400;
        double threshold=200; 
        double minLineLength=200; 
        double maxLineGap=50;
        //Probabilistic Hough Transform:
        HoughLinesP(edges, lines, rho, theta, threshold, minLineLength, maxLineGap);
        N = lines.size();
        myprintf("N:%d\n", fn, N);
        for (int i=0; i<N; i++)
        {
            int x1=lines[i][0];
            int y1=lines[i][1];
            int x2=lines[i][2];
            int y2=lines[i][3];
            myprintf("x1,y1:%d,%d x2,y2:%d,%d\n", fn, x1,y1,x2,y2);
            line(normalizedFrame, Point(x1, y1), Point(x2, y2), Scalar(255, 0, 0), 2);
        }
        display_image(normalizedFrame);
        //for i in range(N):
        //    x1 = lines[i][0][0]
        //    y1 = lines[i][0][1]
        //    x2 = lines[i][0][2]
        //    y2 = lines[i][0][3]
        //    cv2.line(normalizedFrame, (x1, y1), (x2, y2), (255, 0, 0), 2)
    }
    else
    {
        // Regular Transform
        vector<Vec2f> lines;
        double Rres = 1;
        double Thetares = 1 * CV_PI / 400;
        HoughLines(edges, lines, Rres, Thetares, 300);
        
        N = lines.size();
        if(N!=0)
        {
            N = min(N,5);
            myprintf("lines num:%d\n", fn, N);
            vector<Vec2f> strong_lines;
            get_strong_lines(lines,strong_lines, 100);
            for (int i=0; i<strong_lines.size(); i++)
            {
                Vec2f my_lines=strong_lines[i];
                float rho = my_lines[0];
                float theta = my_lines[1];
                if ((rho==0) && (theta==0))
                {
                    break;
                }
                else
                {
                    myprintf("%f, %f\n", fn, rho, theta);
                    float a = cos(theta);
                    float b = sin(theta);
                    float x0 = a * rho;
                    float y0 = b * rho;
                    int x1 = int(x0 + 1000 * (-b));
                    int y1 = int(y0 + 1000 * (a));
                    int x2 = int(x0 - 1000 * (-b));
                    int y2 = int(y0 - 1000 * (a));

                    line(normalizedFrame, Point(x1, y1), Point(x2, y2), Scalar(255, 0, 0), 2);
                }
            }

        }
        display_image(normalizedFrame);
    }
 
}
/******************
 * Function: main
 * ****************/
int main(int argc, char** argv )
{
    const char* fn="main()";
    if ( argc != 2 )
    {
        myprintf("usage: %s <Image_Path>\n", fn, argv[0]);
        return -1;
    }

    Mat image;
    image = imread( argv[1], 1 );

    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    find_line(image);
    #if 0
    namedWindow("Display Image", WINDOW_AUTOSIZE );
    imshow("Display Image", image);

    waitKey(0);
    #endif

    return 0;
}
/**********************
 * display_image
 * ********************/
void display_image(Mat img)
{
    namedWindow("Display Image", WINDOW_AUTOSIZE );
    imshow("Display Image", img);
    waitKey(0);
}
/***********************
 * get_strong_lines
 *  A function used to extract distinct lines from a HoughTransform lines list

 * Attributes:
 * ----------
 * lines - a list of (rho, theta)
 * rho_min_dist - the min dist between distinct lines
 
 * Output:
 * ------
 * strong_lines - a structure including up to 4 distinct lines
 * *********************/
int get_strong_lines(vector<Vec2f>& lines, vector<Vec2f>& strong_lines, float rho_min_dst)
{
    const char* fn="get_strong_lines()";
    int ret=0;
    int n2 = 0;
    int max_lines_to_analyze = 10;
    int N = min(max_lines_to_analyze,int(lines.size()));
    for (int n1=0; n1<N; n1++)
    {
        float rho=lines[n1][0];
        float theta=lines[n1][1];
        if (n1 == 0)
        {
            strong_lines.push_back(lines[n1]);
            //print(lines[n1])
            n2 = n2 + 1;
        }
        else
        {
            //Handle singularity point
            if (rho < 0)
            {
               rho*=-1;
               theta-=CV_PI;
            }
            // Check if line is close to previous lines
            for (int j=0; j<n2;j++)
            {
                if (strong_lines[j][0] < rho)
                {
                    break;
                } 
                if (strong_lines[j][1] < CV_PI/36)
                {
                    break;
                }
                if (n2 < 4)
                {
                    strong_lines.push_back(lines[n1]);
                    n2+=1;
                } 
             }
        }


    }

   
    return (ret);
} 
