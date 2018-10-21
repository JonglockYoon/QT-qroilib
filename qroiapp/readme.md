qroiapp compile step

----
<pre>
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

----

/usr/local/include/tesseract/tesscallback.h:278: error: ‘remove_reference’ is not a class template
 template<typename T> struct remove_reference<T&> { typedef T type; };
                             ^
When a tesseract compile error occurs, comment the 3 line sentence as shown below.

// Specified by TR1 [4.7.2] Reference modifications.
//template <class T> struct remove_reference;
//template<typename T> struct remove_reference { typedef T type; };
//template<typename T> struct remove_reference<T&> { typedef T type; };



</pre>

