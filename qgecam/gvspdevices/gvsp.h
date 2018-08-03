/***************************************************************************
 *   Copyright (C) 2014-2017 by Cyril BALETAUD                             *
 *   cyril.baletaud@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef GVSP_H
#define GVSP_H

#include <type_traits>

//===================================================
// PIXEL FORMATS
//===================================================
// Indicate if pixel is monochrome or RGB
#define GVSP_PIX_MONO           0x01000000
#define GVSP_PIX_RGB            0x02000000 // deprecated in version 1.1
#define GVSP_PIX_COLOR          0x02000000
#define GVSP_PIX_CUSTOM         0x80000000
#define GVSP_PIX_COLOR_MASK     0xFF000000
// Indicate effective number of bits occupied by the pixel (including padding).
// This can be used to compute amount of memory required to store an image.
#define GVSP_PIX_OCCUPY8BIT     0x00080000
#define GVSP_PIX_OCCUPY12BIT    0x000C0000
#define GVSP_PIX_OCCUPY16BIT    0x00100000
#define GVSP_PIX_OCCUPY24BIT    0x00180000
#define GVSP_PIX_OCCUPY32BIT    0x00200000
#define GVSP_PIX_OCCUPY36BIT    0x00240000
#define GVSP_PIX_OCCUPY48BIT    0x00300000
#define GVSP_PIX_EFFECTIVE_PIXEL_SIZE_MASK  0x00FF0000
#define GVSP_PIX_EFFECTIVE_PIXEL_SIZE_SHIFT 16
// Pixel ID: lower 16-bit of the pixel formats
#define GVSP_PIX_ID_MASK        0x0000FFFF
// Mono buffer format defines
#define GVSP_PIX_MONO8          (GVSP_PIX_MONO|GVSP_PIX_OCCUPY8BIT|0x0001)
#define GVSP_PIX_MONO8_SIGNED   (GVSP_PIX_MONO|GVSP_PIX_OCCUPY8BIT|0x0002)
#define GVSP_PIX_MONO10         (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x0003)
#define GVSP_PIX_MONO10_PACKED  (GVSP_PIX_MONO|GVSP_PIX_OCCUPY12BIT|0x0004)
#define GVSP_PIX_MONO12         (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x0005)
#define GVSP_PIX_MONO12_PACKED  (GVSP_PIX_MONO|GVSP_PIX_OCCUPY12BIT|0x0006)
#define GVSP_PIX_MONO14         (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x0025)
#define GVSP_PIX_MONO16         (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x0007)
// Bayer buffer format defines
#define GVSP_PIX_BAYGR8         (GVSP_PIX_MONO|GVSP_PIX_OCCUPY8BIT|0x0008)
#define GVSP_PIX_BAYRG8         (GVSP_PIX_MONO|GVSP_PIX_OCCUPY8BIT|0x0009)
#define GVSP_PIX_BAYGB8         (GVSP_PIX_MONO|GVSP_PIX_OCCUPY8BIT|0x000A)
#define GVSP_PIX_BAYBG8         (GVSP_PIX_MONO|GVSP_PIX_OCCUPY8BIT|0x000B)
#define GVSP_PIX_BAYGR10        (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x000C)
#define GVSP_PIX_BAYRG10        (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x000D)
#define GVSP_PIX_BAYGB10        (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x000E)
#define GVSP_PIX_BAYBG10        (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x000F)
#define GVSP_PIX_BAYGR12        (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x0010)
#define GVSP_PIX_BAYRG12        (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x0011)
#define GVSP_PIX_BAYGB12        (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x0012)
#define GVSP_PIX_BAYBG12        (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x0013)
#define GVSP_PIX_BAYGR10_PACKED (GVSP_PIX_MONO|GVSP_PIX_OCCUPY12BIT|0x0026)
#define GVSP_PIX_BAYRG10_PACKED (GVSP_PIX_MONO|GVSP_PIX_OCCUPY12BIT|0x0027)
#define GVSP_PIX_BAYGB10_PACKED (GVSP_PIX_MONO|GVSP_PIX_OCCUPY12BIT|0x0028)
#define GVSP_PIX_BAYBG10_PACKED (GVSP_PIX_MONO|GVSP_PIX_OCCUPY12BIT|0x0029)
#define GVSP_PIX_BAYGR12_PACKED (GVSP_PIX_MONO|GVSP_PIX_OCCUPY12BIT|0x002A)
#define GVSP_PIX_BAYRG12_PACKED (GVSP_PIX_MONO|GVSP_PIX_OCCUPY12BIT|0x002B)
#define GVSP_PIX_BAYGB12_PACKED (GVSP_PIX_MONO|GVSP_PIX_OCCUPY12BIT|0x002C)
#define GVSP_PIX_BAYBG12_PACKED (GVSP_PIX_MONO|GVSP_PIX_OCCUPY12BIT|0x002D)
#define GVSP_PIX_BAYGR16        (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x002E)
#define GVSP_PIX_BAYRG16        (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x002F)
#define GVSP_PIX_BAYGB16        (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x0030)
#define GVSP_PIX_BAYBG16        (GVSP_PIX_MONO|GVSP_PIX_OCCUPY16BIT|0x0031)
// RGB Packed buffer format defines
#define GVSP_PIX_RGB8_PACKED    (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY24BIT|0x0014)
#define GVSP_PIX_BGR8_PACKED    (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY24BIT|0x0015)
#define GVSP_PIX_RGBA8_PACKED   (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY32BIT|0x0016)
#define GVSP_PIX_BGRA8_PACKED   (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY32BIT|0x0017)
#define GVSP_PIX_RGB10_PACKED   (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY48BIT|0x0018)
#define GVSP_PIX_BGR10_PACKED   (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY48BIT|0x0019)
#define GVSP_PIX_RGB12_PACKED   (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY48BIT|0x001A)
#define GVSP_PIX_BGR12_PACKED   (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY48BIT|0x001B)
#define GVSP_PIX_RGB16_PACKED   (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY48BIT|0x0033)
#define GVSP_PIX_RGB10V1_PACKED (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY32BIT|0x001C)
#define GVSP_PIX_RGB10V2_PACKED (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY32BIT|0x001D)
#define GVSP_PIX_RGB12V1_PACKED (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY36BIT|0X0034)
#define GVSP_PIX_RGB565_PACKED  (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY16BIT|0x0035)
#define GVSP_PIX_BGR565_PACKED  (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY16BIT|0X0036)
// YUV Packed buffer format defines
#define GVSP_PIX_YUV411_PACKED  (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY12BIT|0x001E)
#define GVSP_PIX_YUV422_PACKED  (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY16BIT|0x001F)
#define GVSP_PIX_YUV422_YUYV_PACKED (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY16BIT|0x0032)
#define GVSP_PIX_YUV444_PACKED  (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY24BIT|0x0020)
// RGB Planar buffer format defines
#define GVSP_PIX_RGB8_PLANAR    (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY24BIT|0x0021)
#define GVSP_PIX_RGB10_PLANAR   (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY48BIT|0x0022)
#define GVSP_PIX_RGB12_PLANAR   (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY48BIT|0x0023)
#define GVSP_PIX_RGB16_PLANAR   (GVSP_PIX_COLOR|GVSP_PIX_OCCUPY48BIT|0x0024)

#define GVSP_PIX_PIXEL_SIZE( GVSP_PIX ) (( GVSP_PIX_EFFECTIVE_PIXEL_SIZE_MASK & GVSP_PIX ) >> GVSP_PIX_EFFECTIVE_PIXEL_SIZE_SHIFT )

namespace Jgv
{

namespace Gvsp
{

enum class Status: uint16_t {
    Succes = 0x0000,
    Resend = 0x0100
};

template <typename T>
typename std::underlying_type<T>::type enumType(T enumerator)  {
    return static_cast<typename std::underlying_type<T>::type>(enumerator);
}

} // namespace Gvsp

} // namespace Jgv

#endif // GVSP_H
