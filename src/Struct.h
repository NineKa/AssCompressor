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
#ifndef AssCompressor_Struct_h
#define AssCompressor_Struct_h

#include <string>
#include <vector>

using std::string;
using std::vector;

struct AssConfig{
    string XMLContent,XMLUrl,ContentType,ExportLocation;
    bool ACCEnable,TOPEnable,BOTEnable,ROLEnable;
    double AlphaTOP,AlphaBOT,AlphaROL,AlphaACC;
    long int layerACC,layerTOP,layerBOT,layerROL;
    long int DisplayX,DisplayY;
    long int iframeX,iframeY;
    string FontNameHei,FontNameYouYuan,FontNameSong,
           FontNameKai,FontNameMSHei;
    bool MultilineComment,MultilineCompare;
    bool FollowACCFontName;
    bool Bold,ACCAutoAdjust,ACCFixRotate;
    double MovingVelocity,DynamicLengthRatio;
    long int DefaultFontSize;
    string DefaultFontName;
    string VideoRatio;
    vector<string> remove_color;
    vector<string> remove_user;
    vector<string> remove_key;
    vector<string> remove_pool;
    bool Allow_VisitorComment;
    long comment_at_timestamp;
};

struct Comment_struct{
    double time,timeout;
    long int type;
    long int font_size;
    long int color;
    long int timestamp;
    long int pool;
    string user_id,comment_id;
    string content;
    double leftin,leftout,rightin,rightout;

    long int x0,y0,x1,y1;
    double fadein,fadeout;
    double duration;
    long int z_rotate,y_rotate;
    bool linear_acceleration;
    string fontname;
    bool border_enhance;
    string archer,supplement;
    long int enter,layer;
    double Moving_Pause,Moving_Duration;
    long int max_length_line;
};

#endif
