/*
 AssCompressor - AssCompressor Designed For Project BiliBuffer
 Copyright (C) 2014  CHOSX RecursiveG
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef __ColorIO_Header
#define __ColorIO_Header

#include <string>

using namespace std;

#define _Clear "\e[0m"

enum ColorIO_Color{Black,Red,Green,Yellow,Blue,Magenta,Cyan,White};
enum ColorIO_Style{None,Highlight,Underline,Blink,Reverse,Hidden};

#define ColorIO_Def_Front_Black     "30"
#define ColorIO_Def_Front_Red       "31"
#define ColorIO_Def_Front_Green     "32"
#define ColorIO_Def_Front_Yellow    "33"
#define ColorIO_Def_Front_Blue      "34"
#define ColorIO_Def_Front_Magenta   "35"
#define ColorIO_Def_Front_Cyan      "36"
#define ColorIO_Def_Front_White     "37"

#define ColorIO_Def_Back_Black      "40"
#define ColorIO_Def_Back_Red        "41"
#define ColorIO_Def_Back_Green      "42"
#define ColorIO_Def_Back_Yellow     "43"
#define ColorIO_Def_Back_Blue       "44"
#define ColorIO_Def_Back_Magenta    "45"
#define ColorIO_Def_Back_Cyan       "46"
#define ColorIO_Def_Back_White      "47"

#define ColorIO_Def_Style_None      "0"
#define ColorIO_Def_Style_Highlight "1"
#define ColorIO_Def_Style_Underline "4"
#define ColorIO_Def_Style_Blink     "5"
#define ColorIO_Def_Style_Reverse   "7"
#define ColorIO_Def_Style_Hidden    "8"

string ColorIO_Return_Front_Color(ColorIO_Color Front_Color){
    switch (Front_Color) {
        case Black:
            return ColorIO_Def_Front_Black;
        case Red:
            return ColorIO_Def_Front_Red;
        case Green:
            return ColorIO_Def_Front_Green;
        case Yellow:
            return ColorIO_Def_Front_Yellow;
        case Blue:
            return ColorIO_Def_Front_Blue;
        case Magenta:
            return ColorIO_Def_Front_Magenta;
        case Cyan:
            return ColorIO_Def_Front_Cyan;
        case White:
            return ColorIO_Def_Front_White;
    }
}

string ColorIO_Return_Back_Color(ColorIO_Color Back_Color){
    switch (Back_Color) {
        case Black:
            return ColorIO_Def_Back_Black;
        case Red:
            return ColorIO_Def_Back_Red;
        case Green:
            return ColorIO_Def_Back_Green;
        case Yellow:
            return ColorIO_Def_Back_Yellow;
        case Blue:
            return ColorIO_Def_Back_Blue;
        case Magenta:
            return ColorIO_Def_Back_Magenta;
        case Cyan:
            return ColorIO_Def_Back_Cyan;
        case White:
            return ColorIO_Def_Back_White;
    }
}

string ColorIO_Return_Style(ColorIO_Style Style){
    switch (Style) {
        case None:
            return ColorIO_Def_Style_None;
        case Highlight:
            return ColorIO_Def_Style_Highlight;
        case Underline:
            return ColorIO_Def_Style_Underline;
        case Blink:
            return ColorIO_Def_Style_Blink;
        case Reverse:
            return ColorIO_Def_Style_Reverse;
        case Hidden:
            return ColorIO_Def_Style_Hidden;
    }
}

string Colorize(string content,ColorIO_Color Front_Color,ColorIO_Color Back_Color,ColorIO_Style Style){
    return "\e["+ColorIO_Return_Front_Color(Front_Color)+";"+ColorIO_Return_Back_Color(Back_Color)+";"+ColorIO_Return_Style(Style)+"m"+content+_Clear;
}

string Colorize(string content,ColorIO_Color Front_Color,ColorIO_Color Back_Color){
    return "\e["+ColorIO_Return_Front_Color(Front_Color)+";"+ColorIO_Return_Back_Color(Back_Color)+"m"+content+_Clear;
}

string Colorize(string content,ColorIO_Color Front_Color){
    return "\e["+ColorIO_Return_Front_Color(Front_Color)+"m"+content+_Clear;
}

#endif
