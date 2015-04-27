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
 along with this program; if not, write to the Free Sddddsdaoftware
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 
#ifndef __AssCompressor_H__
#define __AssCompressor_H__

#include <sstream>
#include <fstream>
#include "BilibiliCommentManager.h"

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

using namespace std;
#define AssCompressorVersion "1.2.0 [13Dec2014]"
class AssCompressor{
public:
    AssCompressor(AssConfig&);
    string ErrorMsg,ASS;
    void PrintConfig();
private:
    AssConfig &config;           //Store Config Data
    void doConvert_Bilibili_XML();
    void InjectHeader();
    
    string POS_STR(Comment_struct&); //TOP & Bottom comments
    string MOVE_STR(Comment_struct&);//Rolling comments
    string ACC_STR(Comment_struct&);    
    string ASS_TIME(double);
    string HEX_COLOR(long int);
    string ALPHA(long int);
    string BorderSelector(Comment_struct&);
    inline string ASS_Write_Config();
    
    inline string Colorize_Bool(bool);
    inline bool num_string(string&);
    inline string INT2STR(long);
};

#endif//__AssCompressor_H__
