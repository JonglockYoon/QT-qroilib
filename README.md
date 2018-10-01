# qroilib
QT Vision ROI Library(QROILIB_VERSION_STRING=0.0.3)
-----
<pre>
0.0.2 : blobcontour.h update
0.0.3 : update CBlob::JoinBlob()
</pre>
-----

This program has been tested on ARM linux and x86 linux and windows.

This program was created to enable the ROI of 'Machine Vision Program' to be implemented in QT environment.

I used Gwenview's image viewer function and Tiled's object drawing function, and implemented various functions using OpenCV.

-----
<pre>
이 Program은 Machine Vision Program을 QT 환경에서 쉽게 구현할수 있게 하기 위해 작성되었다.

Gwenview 의 Multi view 기능과 메모리보다 큰 이미지를 로드 할 수 있도록 작성된 이미지 viewer기능을 이용하고,
Tiled의 Object drawing 기능을 가져와서 camera의 이미지를 실시간 display하면서 vision ROI를 작성하여,
내가 원하는 결과를 OpenCV를 통하여 쉽게 구현할수 있도록 한다.

이 Program은 ARM계열 linux  와 X86계열 linux 및 windows 에서 test되었다.

다음 블로그에 각 Program의 설명들이 있습니다.
</pre>
https://blog.naver.com/jerry1455/221360187807 - qroilib Simulator (OpenCV code scanner, color detect)
https://blog.naver.com/jerry1455/221359626135 - qroilib Simulator (OpenCV feature detect, matching)
https://blog.naver.com/jerry1455/221355750336 - qroilib Simulator (OpenCV contour extract)
https://blog.naver.com/jerry1455/221351163442 - qroilib Simulator (OpenCV edge detector)
https://blog.naver.com/jerry1455/221347932308 - qroilib Simulator (OpenCV circle fit)
https://blog.naver.com/jerry1455/221347271028 - qroilib Simulator (OpenCV blob)
https://blog.naver.com/jerry1455/221346560824 - qroilib Simulator (OpenCV line fit)
https://blog.naver.com/jerry1455/221341913377 - qroilib Align(OpenCV Align Image)
https://blog.naver.com/jerry1455/221339548001 - qroilib Editor(OpenCV ROI Editor)
https://blog.naver.com/jerry1455/221337051707 - PiCam Capture(OpenCV PiCam Capture)
https://blog.naver.com/jerry1455/221335173223 - Multi Capture Image (OpenCV Multi Capture)
https://blog.naver.com/jerry1455/221332990270 - Capture Image (OpenCV Capture Image)
https://blog.naver.com/jerry1455/221332701399 - qroilib image viewer(OpenCV Image viewer)
https://blog.naver.com/jerry1455/221331480081 - qroilib란(OpenCV ROI Editor Library)?

-----

<pre>
applications using qroilib.

qroiviewer : image viewer using qroilib.
qroicapture : single web camera capture program using qroilib.
qroimulticap : multi web camera capture program using qroilib.
qroipicam : raspberry pi camera capture program using qroilib.
qroieditor : ROI editor program using qroilib.
qroialign : align program using qroilib.
qroisimulator : simulation program using qroilib.
   Threshold,Mopology,Edge,Blob,Contour,Arithmeticlogic
   LineFit,Circle
   CenterOfPlusMark,GeoMatch,SURF/SIFT/ORB Features2D,ColorDetect
qroistereo : OpenCV cv::StereoBM, cv::StereoSGBM test program using qroilib
qgecam : Giga Ethernet Camera Control & Display Program
         I am trying to integrate the jiguiviou project with qroilib.
</pre>
