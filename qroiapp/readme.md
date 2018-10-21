qroiapp compile step

1. make leptonica library
git clone https://github.com/DanBloomberg/leptonica leptonica
cd leptonica
mkdir build
cd build
make ..
sudo make install
2. make tesseract library
git clone https://github.com/tesseract-ocr/tesseract tesseract
cd tesseract
mkdir build
cd build
make ..
sudo make install
3. qroiapp 
open qroilib.pro using qtcreator
