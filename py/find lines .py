#!/usr/bin/env python
import numpy as np
import cv2

Probabilistic = False
is_camera=True
def get_strong_lines(lines,rho_min_dist):

    """A function used to extract distinct lines from a HoughTransform lines list

    Attributes:
    ----------
    lines - a list of (rho, theta)
    rho_min_dist - the min dist between distinct lines

    Output:
    ------
    strong_lines - a structure including up to 4 distinct lines
    """

    # Init strong lines structure
    strong_lines = np.zeros([4,1,2])

    n2 = 0
    max_lines_to_analyze = 10
    N = min(max_lines_to_analyze,len(lines))

    for n1 in range(0,N):
        for rho,theta in lines[n1]:
            if n1 == 0:
                strong_lines[n2] = lines[n1]
                #print(lines[n1])
                n2 = n2 + 1
            else:
                # Handle singularity point
                if rho < 0:
                   rho*=-1
                   theta-=np.pi

                # Check if line is close to previous lines
                closeness_rho = np.isclose(rho,strong_lines[0:n2,0,0],atol = rho_min_dist)
                closeness_theta = np.isclose(theta,strong_lines[0:n2,0,1],atol = np.pi/36)
                closeness = np.all([closeness_rho,closeness_theta],axis=0)
                if not any(closeness) and n2 < 4:
                    strong_lines[n2] = lines[n1]
                    n2 = n2 + 1
                    #print(lines[n1])
    return strong_lines

def find_line(cap=None, frame=None, out_file=None):

    """A function used to find distinct lines in an images stream

    Attributes:
    ----------
    cap -
    rho_min_dist - the min dist between distinct lines

    Output:
    ------
    strong_lines - a structure including up to 4 distinct lines
    """

    while(1):
        
        # Read next image from file
        if cap!=None:
            if not cap.isOpened():
                break;
            ret, frame = cap.read()
        else:
            ret=1;
        # If video file has ended -> break
        if not(ret):
            break

        # Normalize the input image
        height, width = frame.shape[:2]
        #print(width, height, 'width', 'height')
        normalizedFrame = np.zeros((height, width))
        normalizedFrame = cv2.normalize(frame, normalizedFrame, 0, 255, cv2.NORM_MINMAX)
        #cv2.imshow('dst_rt', normalizedImg)

        # Translate from CV2 BGR to gray scale
        gray = cv2.cvtColor(normalizedFrame, cv2.COLOR_BGR2GRAY)
        # crop_img = gray[(height * 0.1:200, 0:-1]

        # Canny Edge Detection:
        C_Threshold1 = 20; # 200 # pixels with lower gradient are declared not-edge
        C_Threshold2 = 50; # 500 # pixels with higher gradient are declared edge
        edges = cv2.Canny(gray, C_Threshold1, C_Threshold2, apertureSize=3)
        #cv2.imshow('dst_rt', edges)

        # Hough Transform
        if Probabilistic:
            # Probabilistic Hough Transform:
            lines = cv2.HoughLinesP(edges, rho=1, theta=1 * np.pi / 400, threshold=200, minLineLength=200, maxLineGap=50)
            #lines = cv2.HoughLinesP(edges, rho=1, theta=1 * np.pi / 180, threshold=100, minLineLength=100, maxLineGap=50)
            N = lines.shape[0]
            for i in range(N):
                x1 = lines[i][0][0]
                y1 = lines[i][0][1]
                x2 = lines[i][0][2]
                y2 = lines[i][0][3]
                cv2.line(normalizedFrame, (x1, y1), (x2, y2), (255, 0, 0), 2)
        else:
            # Regular Transform
            Rres = 1
            Thetares = 1 * np.pi / 400
            lines = cv2.HoughLines(edges, Rres, Thetares, 300)

            if not(lines is None):
                N = min(len(lines),5)
                print('lines num =', N)

                strong_lines = get_strong_lines(lines,100)
                for my_lines in strong_lines:
                    rho = my_lines[0][0]
                    theta = my_lines[0][1]
                    if (rho==0) and (theta==0):
                        break;
                    else:
                        print(rho, theta)
                        a = np.cos(theta)
                        b = np.sin(theta)
                        x0 = a * rho
                        y0 = b * rho
                        x1 = int(x0 + 1000 * (-b))
                        y1 = int(y0 + 1000 * (a))
                        x2 = int(x0 - 1000 * (-b))
                        y2 = int(y0 - 1000 * (a))

                        cv2.line(normalizedFrame, (x1, y1), (x2, y2), (255, 0, 0), 2)

                '''
                for rho, theta in lines[0:N,0]:
                    a = np.cos(theta)
                    b = np.sin(theta)
                    x0 = a * rho
                    y0 = b * rho
                    x1 = int(x0 + 1000 * (-b))
                    y1 = int(y0 + 1000 * (a))
                    x2 = int(x0 - 1000 * (-b))
                    y2 = int(y0 - 1000 * (a))
                    print(rho, theta)
                    
                    cv2.line(normalizedFrame, (x1, y1), (x2, y2), (255, 0, 0), 2)
                    '''
        if out_file !=None:
            out_file.write(normalizedFrame)
        cv2.imshow('frame', normalizedFrame)

        if cv2.waitKey(5) & 0xFF == ord('q'):
            break



########################################################
# main
########################################################

def main():
    #cap = cv2.VideoCapture('deck vid.mp4')
    if is_camera:
        cap = cv2.VideoCapture('gals deck.mp4')

        fourcc = cv2.VideoWriter_fourcc(*'XVID')
        out = cv2.VideoWriter('output.avi', fourcc, 20.0, (640, 480))
        find_line(cap=cap, frame=None, out_file=out)
    else:
        frame = cv2.imread("../images/frame-1.png")

        find_line(cap=None, frame=frame, out_file=None)

    if is_camera:
        cap.release()
    cv2.destroyAllWindows()

if __name__ == '__main__':
    main()
