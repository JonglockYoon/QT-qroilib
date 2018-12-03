# qroilib
QT Vision ROI Library(QROILIB_VERSION_STRING=1.0.1)
-----
<pre>
OpenCV : 3.4.0 
Linux : QT-5.9.0
Windows : QT-5.9.0
          Visial studio 2015
-----
0.0.2 : blobcontour.h update
0.0.3 : update CBlob::JoinBlob()
0.0.4 : update BlobLib
0.0.6 : update CImgProcBase::FilterLargeDiameter()
0.0.7 : update CImgProcBase::AffineTransform()
0.0.8 : update OrthogonalRenderer::drawRoiObject()
0.0.9 : update FillBlob parameter
1.0.1 : change template image gray to color
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
https://blog.naver.com/jerry1455/221405996453 - qroiapp Color Matching Rate(OpenCV 칼라매칭율)
https://blog.naver.com/jerry1455/221405305176 - qroiapp Color Matching(OpenCV 칼라매칭)
https://blog.naver.com/jerry1455/221404801140 - qroilib Simulator calcBackProject(OpenCV 히스토그램 역투영)
https://blog.naver.com/jerry1455/221401937306 - qroilib Simulator 히스토그램평활화(CLAHE)
https://blog.naver.com/jerry1455/221400989043 - qroilibapp Line Measurement with OpenCV(거리측정)
https://blog.naver.com/jerry1455/221396435441 - qroilibapp 패턴매칭 과 피처매칭 속도비교 with OpenCV
https://blog.naver.com/jerry1455/221389173153 - qroilibapp 피처매칭(feature matching) with OpenCV
https://blog.naver.com/jerry1455/221388359589 - qroilibapp 템플릿매칭(패턴매칭) with OpenCV
https://blog.naver.com/jerry1455/221387457617 - qroilibapp barcode decoding with QZXing
https://blog.naver.com/jerry1455/221386839273 - qroilibapp OpenCV camera capture image semaphore 도입
https://blog.naver.com/jerry1455/221385871213 - qroilibapp 구조 with OpenCV
https://blog.naver.com/jerry1455/221382069339 - qroilibapp tesseract with OpenCV
https://blog.naver.com/jerry1455/221377504552 - qroilib simulator ImageSegmentation (distanceTransform,watershed)
https://blog.naver.com/jerry1455/221376401327 - qroilib simulator user’s manual
https://blog.naver.com/jerry1455/221372070859 - qroilib Simulator (OpenCV edge application)
https://blog.naver.com/jerry1455/221371946121 - qroilib compile and run on Raspberry pi
https://blog.naver.com/jerry1455/221370404281 - qroilib ImgProcBase class APIs
https://blog.naver.com/jerry1455/221370304964 - qroilib compile and run on ubuntu 18.04
https://blog.naver.com/jerry1455/221369110092 - qroilib Simulator (OpenCV FFT, Pattern Erase)
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

https://blog.naver.com/jerry1455/221407563931 - OpenCV-4.0.0 compile.
https://blog.naver.com/jerry1455/221404800504 - OpenCV #13 copyMakeBorder.
https://blog.naver.com/jerry1455/221404202816 - OpenCV #12 WebCam AutoExposure.
https://blog.naver.com/jerry1455/221403428262 - OpenCV #11 자동 초점 기능(Auto Focus)
https://blog.naver.com/jerry1455/221402622851 - OpenCV #10 파노라마 Stitching (Stitcher)
https://blog.naver.com/jerry1455/221400549618 - OpenCV #9 blob feature (2)
https://blog.naver.com/jerry1455/221400510903 - OpenCV #8 Contours 응용(2)
https://blog.naver.com/jerry1455/221397184991 - OpenCV #7 Contours 응용.
https://blog.naver.com/jerry1455/221389104013 - OpenCV #6 패턴 및 피처 매칭(Pattern & Feature Matching)
https://blog.naver.com/jerry1455/221368080208 - OpenCV #5 OpenCV Blob APIs.
https://blog.naver.com/jerry1455/221365405520 - OpenCV #4 blob feature.
https://blog.naver.com/jerry1455/221359367760 - OpenCV #3 LineFit ,EdgeCorner,FindShape,CenterOfBlob.
https://blog.naver.com/jerry1455/221354695256 - OpenCV #2 Canny,Sobel,Laplacian.
https://blog.naver.com/jerry1455/221341480823 - OpenCV #1 Binarization,Morphology,Arithmetic.

-----

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

