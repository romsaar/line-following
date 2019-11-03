import numpy as np
import cv2

cap = cv2.VideoCapture('gals deck.mp4')

while(cap.isOpened()):
    ret, frame = cap.read()

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    height, width = frame.shape[:2]
    print(width, height, 'width', 'height')


   # crop_img = gray[(height * 0.1:200, 0:-1]
    edges = cv2.Canny(gray, 100, 150, apertureSize=3)

    lines = cv2.HoughLines(edges, 1, np.pi / 400, 300)

    for rho, theta in lines[0]:
        a = np.cos(theta)
        b = np.sin(theta)
        x0 = a * rho
        y0 = b * rho
        x1 = int(x0 + 1000 * (-b))
        y1 = int(y0 + 1000 * (a))
        x2 = int(x0 - 1000 * (-b))
        y2 = int(y0 - 1000 * (a))

        cv2.line(frame, (x1, y1), (x2, y2), (0, 0, 255), 25)

    cv2.imshow('frame',frame)
    if cv2.waitKey(50) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
