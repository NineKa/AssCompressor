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
#ifndef __AssCompressor__BilibiliCommentManager__
#define __AssCompressor__BilibiliCommentManager__
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <math.h>
#include <climits>
#include <cstring>
#include <curl/curl.h>
#include "rapidxml/RapidXML.h"
#include "Struct.h"
#define BilibiliFont_KEY_HEI      "黑体"
#define BilibiliFont_KEY_YouYuan  "幼圆"
#define BilibiliFont_KEY_Song     "宋体"
#define BilibiliFont_KEY_Kai      "楷体"
#define BilibiliFont_KEY_MS_HEI   "微软雅黑"

//Macro for Bilibili_Comment_Type
#define Bilibili_Survival_Time 5
#define Bilibili_ROLL          1
#define Bilibili_TOP           5
#define Bilibili_BOTTOM        4
#define Bilibili_ACC           7

#define Bilibili_Connection_Timeout   120

//Macro for Bilibili_Comment_Regex
#define Bilibili_Comment_Error         "\"></d>"
#define Bilibili_Comment_Regex_Search  "<d p=\"(.+?)\">(.+?)</d>"
#define Bilibili_Comment_Regex_Replace "<d p=\"$1\"><![CDATA[$2]]></d>"
//#define Bilibili_Comment_Regex_Replace "<d p=\"$1\"> $2 </d>"
#define Bilibili_Comment_Line_End      '\n'

//Macro For Warning Guidence
#define Bilibili_Warning_Init_Error    ";Error Encountered When Init Comment:"
#define Bilibili_Warning_ACC15         ";[Coming Soon]Unsupported for format ACC-15:"
#define Bilibili_Warning_Ratio_Error   ";Unable to determine the ratio for "
#define Bilibili_Warning_Font_Error    ";Unable to determine the fontname for "

#define BILIBILI_NO_SPECIFIC_TIMESTAMP -1L

//using namespace boost;
using namespace std;

class BilibiliCommentManager{
public:
    BilibiliCommentManager(AssConfig&);
    void Analysis();
    vector<Comment_struct> &FetchData();
    long int FetchNumComment();
    vector<string> Warning_Message;
private:
    AssConfig &configure;
    long int total_comment;
    string comment_xml_content;
    vector<Comment_struct>Comment_Request;
    vector<double>Dynamic_ROLL;
    vector<double>Dynamic_TOP;
    vector<double>Dynamic_BOTTOM;
    
    long int str2int(string);
    double str2double(string);
    double surval_time();
    void StringSplit(string,char,vector<string>&);
    vector<string> StringSplit(string,char);
    string destr(string&);
    
    void SortRequest(vector<Comment_struct>&);
    void FixBilibiliCommentXML();
    void Comment_Analysis_Single_Normal(string,string,Comment_struct &);
    void Comment_Analysis_Single_Normal_ACC(Comment_struct&);
    void Comment_Analysis_Single_Further_ROLL(Comment_struct&);
    void Comment_Analysis_Single_Further_TOP(Comment_struct&);
    void Comment_Analysis_Single_Further_Bottom(Comment_struct&);
    void Comment_Analysis_Single_Further_ACC(Comment_struct&);
    void Comment_Analysis_Correction_ACC(Comment_struct&);
    void Font_Correction(Comment_struct&);
    long int ROLL_Y(Comment_struct&);
    long int TOP_Y(Comment_struct&);
    long int BOTTOM_Y(Comment_struct&);
    inline long int HeightOccopuy(Comment_struct&);
    inline string HEXColor(long);
    inline string INT2STR(long);
};

#endif /* defined(__AssCompressor__BilibiliCommentManager__) */
