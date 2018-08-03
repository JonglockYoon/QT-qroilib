//-----------------------------------------------------------------------------
//  Company:  Basler Vision Technologies
//  Section:  Vision Components
//  Project:  1394 Driver
//  Subproject:  Color conversion
//  <b> Visual SourceSafe Information:</b>
//  $Archive: /Software/inc/ColorConversion.h $
//  $Author: Happe, A.$
//  $Revision: 8$
//  $Date: 05.06.2003 15:42:53$
//-----------------------------------------------------------------------------
/**
\file     ColorConversion.h
*
* Utility class CColorConversion ( YUV -> RGB and Bayer8 -> RGB conversion)
*
*/
//-----------------------------------------------------------------------------

#include <qglobal.h>

/*

YUV <-> RGB

range : 0 to 255 for Y, R, G, B
-128 to 127 for U and V

R = Y + 1.4022 V
G = Y - 0.3457 U - 0.7144 V
B = Y + 1.7710 U

Y = 0.2989 R + 0.5866 G + 0.1145 B
U = - 0.1688 R - 0.3312 G + 0.5 B
V = 0.5 R - 0.5184 G - 0.0817 B
*/

#pragma once

typedef unsigned char BYTE;
typedef unsigned char *PBYTE;
class CSize
{
public:
    long        cx;
    long        cy;
};

class CColorConversion
{
public:
    /// Convert YUV422 to RGB
    static void ConvertYUV422ToRGB(PBYTE pDest, const PBYTE pSource, const CSize& Size);
    static void ConvertYUV422ToRGB(PBYTE pDest, const PBYTE pLeftSource, const PBYTE pRightSource, const CSize& Size);
//    enum  PatternOrigin_t
//    {
//        poGB = 1,
//        poGR,
//        poB,
//        poR
//    };

//    /// Convert Bayer raw data to RGB
//    static void ConvertMono8ToRGB(PBYTE pDest, const PBYTE pSource, const CSize& Size, PatternOrigin_t PatternOrigin,
//        const BYTE pLutR[256], const BYTE pLutG[256], const BYTE pLutB[256]);



//private:
//    static void ProcessGBLines(RGBTRIPLE* pDest, const PBYTE pSource, const CSize& Size, unsigned int lineoffset,
//        const BYTE pLutR[256], BYTE const pLutG[256], const BYTE pLutB[256]);
//    static void ProcessRGLines(RGBTRIPLE* pDest, const PBYTE pSource, const CSize& Size, unsigned int lineoffset,
//        const BYTE pLutR[256], const BYTE pLutG[256], const BYTE pLutB[256]);
//    static void ProcessBGLines(RGBTRIPLE* pDest, const PBYTE pSource, const CSize& Size, unsigned int lineoffset,
//        const BYTE pLutR[256], const BYTE pLutG[256], const BYTE pLutB[256]);
//    static void ProcessGRLines(RGBTRIPLE* pDest, const PBYTE pSource, const CSize& Size, unsigned int lineoffset,
//        const BYTE pLutR[256], const BYTE pLutG[256], const BYTE pLutB[256]);

};

//------------------------------------------------------------------------------
// void CColorConversion::ConvertYUV422ToRGB(PBYTE pDest, const PBYTE pSource, const CSize& Size)
// Author: 
//------------------------------------------------------------------------------
/**
* Conversion of YUV422 pixel data into RGB 
*
* \param     pDest   Pointer to a buffer the RGB data will be written to
* \param     pSource Pointer to the pixel data to be converted
* \param     Size    Size of image
* \return    void
*
*/
//------------------------------------------------------------------------------

void CColorConversion::ConvertYUV422ToRGB(PBYTE pDest, const PBYTE pSource, const CSize& Size)
{
    static const int LUT_G_U[256] = // ( U - 128 ) * 0.3457 + 0.5
    {-44, -44, -44, -43, -43, -43, -42, -42,
    -41, -41, -41, -40, -40, -40, -39, -39,
    -39, -38, -38, -38, -37, -37, -37, -36,
    -36, -36, -35, -35, -35, -34, -34, -34,
    -33, -33, -32, -32, -32, -31, -31, -31,
    -30, -30, -30, -29, -29, -29, -28, -28,
    -28, -27, -27, -27, -26, -26, -26, -25,
    -25, -25, -24, -24, -24, -23, -23, -22,
    -22, -22, -21, -21, -21, -20, -20, -20,
    -19, -19, -19, -18, -18, -18, -17, -17,
    -17, -16, -16, -16, -15, -15, -15, -14,
    -14, -13, -13, -13, -12, -12, -12, -11,
    -11, -11, -10, -10, -10,  -9,  -9,  -9,
    -8,  -8,  -8,  -7,  -7,  -7,  -6,  -6,
    -6,  -5,  -5,  -4,  -4,  -4,  -3,  -3,
    -3,  -2,  -2,  -2,  -1,  -1,  -1,   0,
    0,   0,   1,   1,   1,   2,   2,   2,
    3,   3,   3,   4,   4,   4,   5,   5,
    6,   6,   6,   7,   7,   7,   8,   8,
    8,   9,   9,   9,  10,  10,  10,  11,
    11,  11,  12,  12,  12,  13,  13,  13,
    14,  14,  15,  15,  15,  16,  16,  16,
    17,  17,  17,  18,  18,  18,  19,  19,
    19,  20,  20,  20,  21,  21,  21,  22,
    22,  22,  23,  23,  24,  24,  24,  25,
    25,  25,  26,  26,  26,  27,  27,  27,
    28,  28,  28,  29,  29,  29,  30,  30,
    30,  31,  31,  31,  32,  32,  32,  33,
    33,  34,  34,  34,  35,  35,  35,  36,
    36,  36,  37,  37,  37,  38,  38,  38,
    39,  39,  39,  40,  40,  40,  41,  41,
    41,  42,  42,  43,  43,  43,  44,  44};

    static const int LUT_G_V[256] = // ( V - 128 ) * 0.7144 + 0.5
    {-91, -91, -90, -89, -89, -88, -87, -86,
    -86, -85, -84, -84, -83, -82, -81, -81,
    -80, -79, -79, -78, -77, -76, -76, -75,
    -74, -74, -73, -72, -71, -71, -70, -69,
    -69, -68, -67, -66, -66, -65, -64, -64,
    -63, -62, -61, -61, -60, -59, -59, -58,
    -57, -56, -56, -55, -54, -54, -53, -52,
    -51, -51, -50, -49, -49, -48, -47, -46,
    -46, -45, -44, -44, -43, -42, -41, -41,
    -40, -39, -39, -38, -37, -36, -36, -35,
    -34, -34, -33, -32, -31, -31, -30, -29,
    -29, -28, -27, -26, -26, -25, -24, -24,
    -23, -22, -21, -21, -20, -19, -19, -18,
    -17, -16, -16, -15, -14, -14, -13, -12,
    -11, -11, -10,  -9,  -9,  -8,  -7,  -6,
    -6,  -5,  -4,  -4,  -3,  -2,  -1,  -1,
    0,   1,   1,   2,   3,   4,   4,   5,
    6,   6,   7,   8,   9,   9,  10,  11,
    11,  12,  13,  14,  14,  15,  16,  16,
    17,  18,  19,  19,  20,  21,  21,  22,
    23,  24,  24,  25,  26,  26,  27,  28,
    29,  29,  30,  31,  31,  32,  33,  34,
    34,  35,  36,  36,  37,  38,  39,  39,
    40,  41,  41,  42,  43,  44,  44,  45,
    46,  46,  47,  48,  49,  49,  50,  51,
    51,  52,  53,  54,  54,  55,  56,  56,
    57,  58,  59,  59,  60,  61,  61,  62,
    63,  64,  64,  65,  66,  66,  67,  68,
    69,  69,  70,  71,  71,  72,  73,  74,
    74,  75,  76,  76,  77,  78,  79,  79,
    80,  81,  81,  82,  83,  84,  84,  85,
    86,  86,  87,  88,  89,  89,  90,  91};

    static const int LUT_B_U[256] = // ( U - 128 ) * 1.7710 + 0.5
    {-227, -225, -223, -221, -220, -218, -216, -214,
    -213, -211, -209, -207, -205, -204, -202, -200,
    -198, -197, -195, -193, -191, -189, -188, -186,
    -184, -182, -181, -179, -177, -175, -174, -172,
    -170, -168, -166, -165, -163, -161, -159, -158,
    -156, -154, -152, -151, -149, -147, -145, -143,
    -142, -140, -138, -136, -135, -133, -131, -129,
    -128, -126, -124, -122, -120, -119, -117, -115,
    -113, -112, -110, -108, -106, -104, -103, -101,
    -99, -97, -96, -94, -92, -90, -89, -87,
    -85, -83, -81, -80, -78, -76, -74, -73,
    -71, -69, -67, -66, -64, -62, -60, -58,
    -57, -55, -53, -51, -50, -48, -46, -44,
    -43, -41, -39, -37, -35, -34, -32, -30,
    -28, -27, -25, -23, -21, -19, -18, -16,
    -14, -12, -11,  -9,  -7,  -5,  -4,  -2,
    0,   2,   4,   5,   7,   9,  11,  12,
    14,  16,  18,  19,  21,  23,  25,  27,
    28,  30,  32,  34,  35,  37,  39,  41,
    43,  44,  46,  48,  50,  51,  53,  55,
    57,  58,  60,  62,  64,  66,  67,  69,
    71,  73,  74,  76,  78,  80,  81,  83,
    85,  87,  89,  90,  92,  94,  96,  97,
    99, 101, 103, 104, 106, 108, 110, 112,
    113, 115, 117, 119, 120, 122, 124, 126,
    128, 129, 131, 133, 135, 136, 138, 140,
    142, 143, 145, 147, 149, 151, 152, 154,
    156, 158, 159, 161, 163, 165, 166, 168,
    170, 172, 174, 175, 177, 179, 181, 182,
    184, 186, 188, 189, 191, 193, 195, 197,
    198, 200, 202, 204, 205, 207, 209, 211,
    213, 214, 216, 218, 220, 221, 223, 225};

    static const int LUT_R_V[256] = // ( V - 128 ) * 1.4022 + 0.5
    {-179, -178, -177, -175, -174, -172, -171, -170,
    -168, -167, -165, -164, -163, -161, -160, -158,
    -157, -156, -154, -153, -151, -150, -149, -147,
    -146, -144, -143, -142, -140, -139, -137, -136,
    -135, -133, -132, -130, -129, -128, -126, -125,
    -123, -122, -121, -119, -118, -116, -115, -114,
    -112, -111, -109, -108, -107, -105, -104, -102,
    -101, -100, -98, -97, -95, -94, -93, -91,
    -90, -88, -87, -86, -84, -83, -81, -80,
    -79, -77, -76, -74, -73, -72, -70, -69,
    -67, -66, -65, -63, -62, -60, -59, -57,
    -56, -55, -53, -52, -50, -49, -48, -46,
    -45, -43, -42, -41, -39, -38, -36, -35,
    -34, -32, -31, -29, -28, -27, -25, -24,
    -22, -21, -20, -18, -17, -15, -14, -13,
    -11, -10,  -8,  -7,  -6,  -4,  -3,  -1,
    0,   1,   3,   4,   6,   7,   8,  10,
    11,  13,  14,  15,  17,  18,  20,  21,
    22,  24,  25,  27,  28,  29,  31,  32,
    34,  35,  36,  38,  39,  41,  42,  43,
    45,  46,  48,  49,  50,  52,  53,  55,
    56,  57,  59,  60,  62,  63,  65,  66,
    67,  69,  70,  72,  73,  74,  76,  77,
    79,  80,  81,  83,  84,  86,  87,  88,
    90,  91,  93,  94,  95,  97,  98, 100,
    101, 102, 104, 105, 107, 108, 109, 111,
    112, 114, 115, 116, 118, 119, 121, 122,
    123, 125, 126, 128, 129, 130, 132, 133,
    135, 136, 137, 139, 140, 142, 143, 144,
    146, 147, 149, 150, 151, 153, 154, 156,
    157, 158, 160, 161, 163, 164, 165, 167,
    168, 170, 171, 172, 174, 175, 177, 178};


    int Y, U, V;
    int R, G, B;
    int deltaG, deltaB, deltaR;
    int i, j, i2, m, n;

    unsigned char* pData;
    unsigned char* pRaw;

    for (i=0; i<Size.cy; i++)
    {
        i2 = i;//Size.cy - i - 1;
        m = 3*i*Size.cx;
        n = 2*i2*Size.cx;
        pData = pDest + m;
        pRaw = (unsigned char*) pSource + n;
        for (j=0; j<Size.cx; j++)

        {	
            U = *pRaw++;
            Y = *pRaw++;
            V = *pRaw++; 
            deltaG = LUT_G_U[U] + LUT_G_V[V];
            deltaR = LUT_R_V[V];
            deltaB = LUT_B_U[U]; 
            R = Y + deltaR;
            G = Y - deltaG;
            B = Y + deltaB;
            if (R < 0) 
                R = 0;
            else if (R > 255) R = 255;
            if (G < 0) 
                G = 0;
            else if (G > 255) 
                G = 255;
            if (B < 0) 
                B = 0;
            else if (B > 255) 
                B = 255; 

            *pData++ = (unsigned char) B;
            *pData++ = (unsigned char) G;
            *pData++ = (unsigned char) R;	
            j++;

            Y = *pRaw++; 

            R = Y + deltaR;
            G = Y - deltaG;
            B = Y + deltaB;
            if (R < 0) 
                R = 0;
            else if (R > 255) R = 255;
            if (G < 0) 
                G = 0;
            else if (G > 255) 
                G = 255;
            if (B < 0) 
                B = 0;
            else if (B > 255) 
                B = 255; 

            *pData++ = (unsigned char) B;
            *pData++ = (unsigned char) G;
            *pData++ = (unsigned char) R;	
        }
    }
}

void CColorConversion::ConvertYUV422ToRGB(PBYTE pDest, const PBYTE pLeftSource, const PBYTE pRightSource, const CSize& Size)
{
    static const int LUT_G_U[256] = // ( U - 128 ) * 0.3457 + 0.5
    {-44, -44, -44, -43, -43, -43, -42, -42,
    -41, -41, -41, -40, -40, -40, -39, -39,
    -39, -38, -38, -38, -37, -37, -37, -36,
    -36, -36, -35, -35, -35, -34, -34, -34,
    -33, -33, -32, -32, -32, -31, -31, -31,
    -30, -30, -30, -29, -29, -29, -28, -28,
    -28, -27, -27, -27, -26, -26, -26, -25,
    -25, -25, -24, -24, -24, -23, -23, -22,
    -22, -22, -21, -21, -21, -20, -20, -20,
    -19, -19, -19, -18, -18, -18, -17, -17,
    -17, -16, -16, -16, -15, -15, -15, -14,
    -14, -13, -13, -13, -12, -12, -12, -11,
    -11, -11, -10, -10, -10,  -9,  -9,  -9,
    -8,  -8,  -8,  -7,  -7,  -7,  -6,  -6,
    -6,  -5,  -5,  -4,  -4,  -4,  -3,  -3,
    -3,  -2,  -2,  -2,  -1,  -1,  -1,   0,
    0,   0,   1,   1,   1,   2,   2,   2,
    3,   3,   3,   4,   4,   4,   5,   5,
    6,   6,   6,   7,   7,   7,   8,   8,
    8,   9,   9,   9,  10,  10,  10,  11,
    11,  11,  12,  12,  12,  13,  13,  13,
    14,  14,  15,  15,  15,  16,  16,  16,
    17,  17,  17,  18,  18,  18,  19,  19,
    19,  20,  20,  20,  21,  21,  21,  22,
    22,  22,  23,  23,  24,  24,  24,  25,
    25,  25,  26,  26,  26,  27,  27,  27,
    28,  28,  28,  29,  29,  29,  30,  30,
    30,  31,  31,  31,  32,  32,  32,  33,
    33,  34,  34,  34,  35,  35,  35,  36,
    36,  36,  37,  37,  37,  38,  38,  38,
    39,  39,  39,  40,  40,  40,  41,  41,
    41,  42,  42,  43,  43,  43,  44,  44};

    static const int LUT_G_V[256] = // ( V - 128 ) * 0.7144 + 0.5
    {-91, -91, -90, -89, -89, -88, -87, -86,
    -86, -85, -84, -84, -83, -82, -81, -81,
    -80, -79, -79, -78, -77, -76, -76, -75,
    -74, -74, -73, -72, -71, -71, -70, -69,
    -69, -68, -67, -66, -66, -65, -64, -64,
    -63, -62, -61, -61, -60, -59, -59, -58,
    -57, -56, -56, -55, -54, -54, -53, -52,
    -51, -51, -50, -49, -49, -48, -47, -46,
    -46, -45, -44, -44, -43, -42, -41, -41,
    -40, -39, -39, -38, -37, -36, -36, -35,
    -34, -34, -33, -32, -31, -31, -30, -29,
    -29, -28, -27, -26, -26, -25, -24, -24,
    -23, -22, -21, -21, -20, -19, -19, -18,
    -17, -16, -16, -15, -14, -14, -13, -12,
    -11, -11, -10,  -9,  -9,  -8,  -7,  -6,
    -6,  -5,  -4,  -4,  -3,  -2,  -1,  -1,
    0,   1,   1,   2,   3,   4,   4,   5,
    6,   6,   7,   8,   9,   9,  10,  11,
    11,  12,  13,  14,  14,  15,  16,  16,
    17,  18,  19,  19,  20,  21,  21,  22,
    23,  24,  24,  25,  26,  26,  27,  28,
    29,  29,  30,  31,  31,  32,  33,  34,
    34,  35,  36,  36,  37,  38,  39,  39,
    40,  41,  41,  42,  43,  44,  44,  45,
    46,  46,  47,  48,  49,  49,  50,  51,
    51,  52,  53,  54,  54,  55,  56,  56,
    57,  58,  59,  59,  60,  61,  61,  62,
    63,  64,  64,  65,  66,  66,  67,  68,
    69,  69,  70,  71,  71,  72,  73,  74,
    74,  75,  76,  76,  77,  78,  79,  79,
    80,  81,  81,  82,  83,  84,  84,  85,
    86,  86,  87,  88,  89,  89,  90,  91};

    static const int LUT_B_U[256] = // ( U - 128 ) * 1.7710 + 0.5
    {-227, -225, -223, -221, -220, -218, -216, -214,
    -213, -211, -209, -207, -205, -204, -202, -200,
    -198, -197, -195, -193, -191, -189, -188, -186,
    -184, -182, -181, -179, -177, -175, -174, -172,
    -170, -168, -166, -165, -163, -161, -159, -158,
    -156, -154, -152, -151, -149, -147, -145, -143,
    -142, -140, -138, -136, -135, -133, -131, -129,
    -128, -126, -124, -122, -120, -119, -117, -115,
    -113, -112, -110, -108, -106, -104, -103, -101,
    -99, -97, -96, -94, -92, -90, -89, -87,
    -85, -83, -81, -80, -78, -76, -74, -73,
    -71, -69, -67, -66, -64, -62, -60, -58,
    -57, -55, -53, -51, -50, -48, -46, -44,
    -43, -41, -39, -37, -35, -34, -32, -30,
    -28, -27, -25, -23, -21, -19, -18, -16,
    -14, -12, -11,  -9,  -7,  -5,  -4,  -2,
    0,   2,   4,   5,   7,   9,  11,  12,
    14,  16,  18,  19,  21,  23,  25,  27,
    28,  30,  32,  34,  35,  37,  39,  41,
    43,  44,  46,  48,  50,  51,  53,  55,
    57,  58,  60,  62,  64,  66,  67,  69,
    71,  73,  74,  76,  78,  80,  81,  83,
    85,  87,  89,  90,  92,  94,  96,  97,
    99, 101, 103, 104, 106, 108, 110, 112,
    113, 115, 117, 119, 120, 122, 124, 126,
    128, 129, 131, 133, 135, 136, 138, 140,
    142, 143, 145, 147, 149, 151, 152, 154,
    156, 158, 159, 161, 163, 165, 166, 168,
    170, 172, 174, 175, 177, 179, 181, 182,
    184, 186, 188, 189, 191, 193, 195, 197,
    198, 200, 202, 204, 205, 207, 209, 211,
    213, 214, 216, 218, 220, 221, 223, 225};

    static const int LUT_R_V[256] = // ( V - 128 ) * 1.4022 + 0.5
    {-179, -178, -177, -175, -174, -172, -171, -170,
    -168, -167, -165, -164, -163, -161, -160, -158,
    -157, -156, -154, -153, -151, -150, -149, -147,
    -146, -144, -143, -142, -140, -139, -137, -136,
    -135, -133, -132, -130, -129, -128, -126, -125,
    -123, -122, -121, -119, -118, -116, -115, -114,
    -112, -111, -109, -108, -107, -105, -104, -102,
    -101, -100, -98, -97, -95, -94, -93, -91,
    -90, -88, -87, -86, -84, -83, -81, -80,
    -79, -77, -76, -74, -73, -72, -70, -69,
    -67, -66, -65, -63, -62, -60, -59, -57,
    -56, -55, -53, -52, -50, -49, -48, -46,
    -45, -43, -42, -41, -39, -38, -36, -35,
    -34, -32, -31, -29, -28, -27, -25, -24,
    -22, -21, -20, -18, -17, -15, -14, -13,
    -11, -10,  -8,  -7,  -6,  -4,  -3,  -1,
    0,   1,   3,   4,   6,   7,   8,  10,
    11,  13,  14,  15,  17,  18,  20,  21,
    22,  24,  25,  27,  28,  29,  31,  32,
    34,  35,  36,  38,  39,  41,  42,  43,
    45,  46,  48,  49,  50,  52,  53,  55,
    56,  57,  59,  60,  62,  63,  65,  66,
    67,  69,  70,  72,  73,  74,  76,  77,
    79,  80,  81,  83,  84,  86,  87,  88,
    90,  91,  93,  94,  95,  97,  98, 100,
    101, 102, 104, 105, 107, 108, 109, 111,
    112, 114, 115, 116, 118, 119, 121, 122,
    123, 125, 126, 128, 129, 130, 132, 133,
    135, 136, 137, 139, 140, 142, 143, 144,
    146, 147, 149, 150, 151, 153, 154, 156,
    157, 158, 160, 161, 163, 164, 165, 167,
    168, 170, 171, 172, 174, 175, 177, 178};


    int Y, U, V;
    int R, G, B;
    int deltaG, deltaB, deltaR;
    int i, j, i2, m, n, m2;
    unsigned char* pData;
    unsigned char* pData2;
    unsigned char* pRaw;

	for(i=0; i<(Size.cx*Size.cy*6); i++) {
		pData = pDest + i;
		*pData = (unsigned char) 0;
	}

	//right area
	for (i=0; i<Size.cy; i++)
	{
		n = 2*i*Size.cx;
		pRaw = (unsigned char*) pRightSource + n;
		for (j=0; j<Size.cx; j++)
		{	
			U = *pRaw++;
			Y = *pRaw++;
			V = *pRaw++; 
			deltaG = LUT_G_U[U] + LUT_G_V[V];
			deltaR = LUT_R_V[V];
			deltaB = LUT_B_U[U]; 
			R = Y + deltaR;
			G = Y - deltaG;
			B = Y + deltaB;
			if (R < 0) 
				R = 0;
			else if (R > 255) R = 255;
			if (G < 0) 
				G = 0;
			else if (G > 255) 
				G = 255;
			if (B < 0) 
				B = 0;
			else if (B > 255) 
				B = 255; 
			m = 3*j*Size.cy*2 + (Size.cy-1-i)*3 + Size.cy*3;
			pData = pDest + m;
			*pData++ = (unsigned char) B;
			*pData++ = (unsigned char) G;
			*pData++ = (unsigned char) R;	
			j++;

			Y = *pRaw++; 

			R = Y + deltaR;
			G = Y - deltaG;
			B = Y + deltaB;
			if (R < 0) 
				R = 0;
			else if (R > 255) R = 255;
			if (G < 0) 
				G = 0;
			else if (G > 255) 
				G = 255;
			if (B < 0) 
				B = 0;
			else if (B > 255) 
				B = 255; 
			m = 3*j*Size.cy*2 + (Size.cy-1-i)*3 + Size.cy*3;
			pData = pDest + m;
			*pData++ = (unsigned char) B;
			*pData++ = (unsigned char) G;
			*pData++ = (unsigned char) R;	
		}
	}

	//left area
	for (i=0; i<Size.cy; i++)
	{
		n = 2*i*Size.cx;
		pRaw = (unsigned char*) pLeftSource + n;
		for (j=0; j<Size.cx; j++)
		{	
			U = *pRaw++;
			Y = *pRaw++;
			V = *pRaw++; 
			deltaG = LUT_G_U[U] + LUT_G_V[V];
			deltaR = LUT_R_V[V];
			deltaB = LUT_B_U[U]; 
			R = Y + deltaR;
			G = Y - deltaG;
			B = Y + deltaB;
			if (R < 0) 
				R = 0;
			else if (R > 255) R = 255;
			if (G < 0) 
				G = 0;
			else if (G > 255) 
				G = 255;
			if (B < 0) 
				B = 0;
			else if (B > 255) 
				B = 255; 
			m = 3*j*Size.cy*2 + (Size.cy-1-i)*3;
			pData = pDest + m;
			*pData++ = (unsigned char) B;
			*pData++ = (unsigned char) G;
			*pData++ = (unsigned char) R;	
			j++;

			Y = *pRaw++; 

			R = Y + deltaR;
			G = Y - deltaG;
			B = Y + deltaB;
			if (R < 0) 
				R = 0;
			else if (R > 255) R = 255;
			if (G < 0) 
				G = 0;
			else if (G > 255) 
				G = 255;
			if (B < 0) 
				B = 0;
			else if (B > 255) 
				B = 255; 
			m = 3*j*Size.cy*2 + (Size.cy-1-i)*3;
			pData = pDest + m;
			*pData++ = (unsigned char) B;
			*pData++ = (unsigned char) G;
			*pData++ = (unsigned char) R;	
		}
	}
}

////------------------------------------------------------------------------------
//// void CColorConversion::ConvertMono8ToRGB(PBYTE pDest, const PBYTE pSource, const CSize& Size, const CPoint& PatternOrigin,  const BYTE const pLutR, const PBYTE const pLutG, const PBYTE const pLutB)
//// Author:
////------------------------------------------------------------------------------
///**
//* Conversion of Bayer raw pixel data to RGB. The user has to specify the origin of the Bayer pattern and to provide
//* lookup tables for the color channels (e.g. for white balance purposes)
//*
//* \param     pDest   Pointer to which the RGB data will be written
//* \param     pSource Pointer to the raw Bayer pixel data to be converted
//* \param     Size    Size of image
//* \param     PatternOrigin Spezifies wether the first pixel of the bayer raw data is a red, blue or green one
//*            ( poGB : green pixel followed by a blue pixel, poGR : green pixel followd by a red pixel,
//poR : red pixel, poB : blue pixel )
//* \param     pLutG Pointer to a lookup table for the green channel
//* \param     pLutB Pointer to a lookup table for the red channel
//* \param     pLutR Pointer to a lookup table for the blue channel
//* \return
//*
//* void
//*
//*/
////------------------------------------------------------------------------------


//void CColorConversion::ConvertMono8ToRGB(PBYTE pDest, const PBYTE pSource, const CSize& Size, PatternOrigin_t PatternOrigin,
//                                         const BYTE pLutR[256], const BYTE pLutG[256], const BYTE pLutB[256])
//{
//    Q_ASSERT ( Size.cx % 2 == 0 && Size.cy % 2 == 0 );
//    Q_ASSERT ( Size.cx > 3);

//    switch ( PatternOrigin)
//    {
//    case poGB:
//        // Bayer Image starts with a GBGB row
//        ProcessGBLines((RGBTRIPLE*) pDest, pSource, Size, 0, pLutR, pLutG, pLutB);
//        ProcessRGLines((RGBTRIPLE*) pDest, pSource, Size, 1, pLutR, pLutG, pLutB);
//        break;
//    case poGR:
//        // Bayer Image starts with a GRGR row
//        ProcessGRLines((RGBTRIPLE*) pDest, pSource, Size, 0, pLutR, pLutG, pLutB);
//        ProcessBGLines((RGBTRIPLE*) pDest, pSource, Size, 1, pLutR, pLutG, pLutB);
//        break;
//    case poB:
//        // Bayer Image starts with a BGBG row
//        ProcessBGLines((RGBTRIPLE*) pDest, pSource, Size, 0, pLutR, pLutG, pLutB);
//        ProcessGRLines((RGBTRIPLE*) pDest, pSource, Size, 1, pLutR, pLutG, pLutB);
//        break;
//    case poR:
//        // Bayer Image starts with a RGRG row
//        ProcessRGLines((RGBTRIPLE*) pDest, pSource, Size, 0, pLutR, pLutG, pLutB);
//        ProcessGBLines((RGBTRIPLE*) pDest, pSource, Size, 1, pLutR, pLutG, pLutB);
//        break;
//    }

//    // Treatment of the border: Erase the right most column and the last row of the destination image
//    RGBTRIPLE *pRGB = (RGBTRIPLE*) pDest;
//    RGBTRIPLE zero = {0, 0, 0};
//    // set the right most column to zero
//    int i;
//    for ( i = 0; i < Size.cy; i++ )
//    {
//        pRGB[(i+1) * Size.cx - 1] = zero;
//    }
//    // set the last row to zero
//    for ( i = 0; i < Size.cx; i ++ )
//    {
//        pRGB[i] = zero;
//    }
//}



//#define REDPIXEL() \
//    pRGB->rgbtBlue = pLutB[*(pRaw + Size.cx + 1)]; \
//    pRGB->rgbtGreen = pLutG[(BYTE) ( ( (int) *(pRaw +1) + (int) *(pRaw + Size.cx) ) >> 1 )]; \
//    pRGB->rgbtRed = pLutR[*pRaw]; \
//    ++pRaw; \
//    ++pRGB;

//#define BLUEPIXEL() \
//    pRGB->rgbtBlue = pLutB[*pRaw]; \
//    pRGB->rgbtGreen = pLutG[(BYTE) ( ( (int) *( pRaw + 1) + (int) *( pRaw + Size.cx ) ) >> 1 )]; \
//    pRGB->rgbtRed = pLutR[*(pRaw + Size.cx + 1)]; \
//    ++pRaw; \
//    ++pRGB;

//#define GREENPIXEL_R() \
//    pRGB->rgbtBlue = pLutB[*(pRaw + Size.cx )]; \
//    pRGB->rgbtGreen = pLutG[*pRaw]; \
//    pRGB->rgbtRed = pLutR[*(pRaw + 1)]; \
//    ++pRaw; \
//    ++pRGB;

//#define GREENPIXEL_B() \
//    pRGB->rgbtBlue = pLutB[*(pRaw + 1)]; \
//    pRGB->rgbtGreen = pLutG[*pRaw]; \
//    pRGB->rgbtRed = pLutR[*(pRaw + Size.cx)]; \
//    ++pRaw; \
//    ++pRGB;

//void CColorConversion::ProcessGBLines(RGBTRIPLE* pDest, const PBYTE pSource, const CSize& Size, unsigned int lineoffset,
//                                      const BYTE pLutR[256], const BYTE pLutG[256], const BYTE pLutB[256])
//{
//    unsigned char* pLastLine = pSource + Size.cx * ( Size.cy  - 1);
//    unsigned char* pRaw = pSource + lineoffset * Size.cx;
//    RGBTRIPLE* pRGB = pDest + Size.cx * (Size.cy - lineoffset - 1);
//    unsigned char* pEnd;
//    while ( pRaw < pLastLine )
//    {
//        pEnd = pRaw + Size.cx - 2;  // we skip the last column
//        while ( pRaw < pEnd )
//        {
//            GREENPIXEL_B();
//            BLUEPIXEL();
//        }
//        GREENPIXEL_B();
//        pRaw += Size.cx + 1;
//        pRGB -= ( 3 * Size.cx - 1 );
//    }
//}

//void CColorConversion::ProcessRGLines(RGBTRIPLE* pDest, const PBYTE pSource, const CSize& Size, unsigned int lineoffset,
//                                      const BYTE pLutR[256], const BYTE pLutG[256], const BYTE pLutB[256])
//{
//    unsigned char* pLastLine = pSource + Size.cx * ( Size.cy  - 1);
//    unsigned char* pRaw = pSource + lineoffset * Size.cx;
//    RGBTRIPLE* pRGB = pDest + Size.cx * (Size.cy - lineoffset - 1);
//    unsigned char* pEnd;
//    while ( pRaw < pLastLine )
//    {
//        pEnd = pRaw + Size.cx - 2;  // we skip the last column
//        while ( pRaw < pEnd )
//        {
//            REDPIXEL();
//            GREENPIXEL_R();
//        }
//        REDPIXEL();
//        pRaw += Size.cx + 1;
//        pRGB -= ( 3 * Size.cx - 1 );
//    }
//}

//void CColorConversion::ProcessBGLines(RGBTRIPLE* pDest, const PBYTE pSource, const CSize& Size, unsigned int lineoffset,
//                                      const BYTE pLutR[256], const BYTE pLutG[256], const BYTE pLutB[256])
//{
//    unsigned char* pLastLine = pSource + Size.cx * ( Size.cy  - 1);
//    unsigned char* pRaw = pSource + lineoffset * Size.cx;
//    RGBTRIPLE* pRGB = pDest + Size.cx * (Size.cy - lineoffset - 1);
//    unsigned char* pEnd;
//    while ( pRaw < pLastLine )
//    {
//        pEnd = pRaw + Size.cx - 2;  // we skip the last column
//        while ( pRaw < pEnd )
//        {
//            BLUEPIXEL();
//            GREENPIXEL_B();
//        }
//        BLUEPIXEL();
//        pRaw += Size.cx + 1;
//        pRGB -= ( 3 * Size.cx - 1 );
//    }
//}

//void CColorConversion::ProcessGRLines(RGBTRIPLE* pDest, const PBYTE pSource, const CSize& Size, unsigned int lineoffset,
//                                      const BYTE pLutR[256], const BYTE pLutG[256], const BYTE pLutB[256])
//{
//    unsigned char* pLastLine = pSource + Size.cx * ( Size.cy  - 1);
//    unsigned char* pRaw = pSource + lineoffset * Size.cx;
//    RGBTRIPLE* pRGB = pDest + Size.cx * (Size.cy - lineoffset - 1);
//    unsigned char* pEnd;
//    while ( pRaw < pLastLine )
//    {
//        pEnd = pRaw + Size.cx - 2;  // we skip the last column
//        while ( pRaw < pEnd )
//        {
//            GREENPIXEL_R();
//            REDPIXEL();
//        }
//        GREENPIXEL_R();
//        pRaw += Size.cx + 1;
//        pRGB -= ( 3 * Size.cx - 1 );
//    }
//}







