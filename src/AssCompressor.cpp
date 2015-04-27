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
#include "AssCompressor.h"

using namespace std;

AssCompressor::AssCompressor(AssConfig &conf):config(conf){
    ErrorMsg="";
    if (config.ContentType=="Bilibili_XML")
        doConvert_Bilibili_XML();
}

string INT2STR(long int val){
    ostringstream out;
    out<<val;
    return out.str();
}

void AssCompressor::PrintConfig() {
    cout<<"XMLUrl:                    "<<config.XMLUrl<<endl;
    cout<<"ACCEnable:                 "<<Colorize_Bool(config.ACCEnable)<<endl;
    cout<<"TOPEnable:                 "<<Colorize_Bool(config.TOPEnable)<<endl;
    cout<<"BOTEnable:                 "<<Colorize_Bool(config.BOTEnable)<<endl;
    cout<<"ROLEnable:                 "<<Colorize_Bool(config.ROLEnable)<<endl;
    cout<<"DisplayX:                  "<<config.DisplayX<<endl;
    cout<<"DisplayY:                  "<<config.DisplayY<<endl;
    cout<<"iframeX:                   "<<config.iframeX<<endl;
    cout<<"iframeY:                   "<<config.iframeY<<endl;
    cout<<"AlphaTOP:                  "<<config.AlphaTOP<<endl;
    cout<<"AlphaBOT:                  "<<config.AlphaBOT<<endl;
    cout<<"AlphaROL:                  "<<config.AlphaROL<<endl;
    cout<<"AlphaACC:                  "<<config.AlphaACC<<endl;
    cout<<"Default_Front_Name:        "<<config.DefaultFontName<<endl;
    cout<<"Default_font_size:         "<<config.DefaultFontSize<<endl;
    cout<<"FollowACCfontname:         "<<Colorize_Bool(config.FollowACCFontName)<<endl;
    cout<<"Moving Velocity:           "<<config.MovingVelocity<<endl;
    cout<<"Video Ratio:               "<<config.VideoRatio<<endl;
    cout<<"Allow Visitor Comment:     "<<Colorize_Bool(config.Allow_VisitorComment)<<endl;
    cout<<"Remove Comment by Color:   "<<Colorize_Bool(config.remove_color.size()!=0)<<endl;
    for (size_t i=0; i<config.remove_color.size(); i++) {
        cout<<"                           "<<(num_string(config.remove_color[i])?HEX_COLOR(atol(config.remove_color[i].c_str())):config.remove_color[i])<<endl;
    }
    cout<<"Remove Comment by User:    "<<Colorize_Bool(config.remove_user.size()!=0)<<endl;
    for (size_t i=0; i<config.remove_user.size(); i++){
        cout<<"                           "<<config.remove_user[i]<<endl;
    }
    cout<<"Remove Comment by Keyword: "<<Colorize_Bool(config.remove_key.size()!=0)<<endl;
    for (size_t i=0; i<config.remove_key.size(); i++){
        cout<<"                           "<<config.remove_key[i]<<endl;
    }
    cout<<"History Comment:           ";
    if (config.comment_at_timestamp==BILIBILI_NO_SPECIFIC_TIMESTAMP) {
        cout<<Colorize_Bool(false)<<endl;
    }else{
        cout<<config.comment_at_timestamp<<endl;
    }
}

string AssCompressor::ASS_TIME(double seconds) {
    int hh=0,mm=0,ss=0,ms=0;
    int ass_seconds;
    ass_seconds=floor(seconds*100);
    ms=ass_seconds%100;
    ass_seconds=floor(ass_seconds/100);
    int int_seconds;
    int_seconds=floor(ass_seconds);
    hh=int_seconds/3600;
    mm=(int_seconds%3600)/60;
    ss=(int_seconds%3600)%60;
    char time_stamp[255];
    sprintf(time_stamp, "%01i:%02i:%02i.%02i",hh,mm,ss,ms);
    return string(time_stamp);
}
string AssCompressor::HEX_COLOR(long int input) {
    char color[7];
    sprintf(color, "%06lx",input);
    string return_str="      ";
    return_str[0]=color[4];return_str[1]=color[5];
    return_str[2]=color[2];return_str[3]=color[3];
    return_str[4]=color[0];return_str[5]=color[1];
    return return_str;
}
string AssCompressor::ALPHA(long int input) {
    char alpha[4];
    sprintf(alpha, "%03lx",input);
    return string(alpha);
}

string AssCompressor::BorderSelector(Comment_struct &obj) {//Need Improve
    if (HEX_COLOR(obj.color)=="000000") {
        return "\\3c&Hffffff";
    }else{
        return "\\3c&H000000";
    }
}
string AssCompressor::POS_STR(Comment_struct &obj) {
    string posstr="";
    string content_correction=obj.content;
    if (config.Bold) {
        content_correction="{\\b1}"+content_correction+"{\\b0}";
    }
    posstr="Dialogue: "+INT2STR(obj.layer)+","+ASS_TIME(obj.time)+","+ASS_TIME(obj.timeout)+",BufferFront,,0000,0000,0000,,{"+obj.archer+BorderSelector(obj)+"\\fs"+INT2STR(obj.font_size)+"\\c&H"+HEX_COLOR(obj.color)+"\\alpha&H"+ALPHA(obj.fadein)+"\\pos("+INT2STR(obj.x0)+","+INT2STR(obj.y0)+")}"+content_correction+"\n";
    return posstr;
}
string AssCompressor::MOVE_STR(Comment_struct &obj) {
    string movestr="";
    string content_correction=obj.content;
    if (config.Bold) {
        content_correction="{\\b1}"+content_correction+"{\\b0}";
    }
    movestr="Dialogue: "+INT2STR(obj.layer)+","+ASS_TIME(obj.time)+","+ASS_TIME(obj.timeout)+",BufferFront,,0000,0000,0000,,{"+obj.archer+BorderSelector(obj)+"\\fs"+INT2STR(obj.font_size)+"\\c&H"+HEX_COLOR(obj.color)+"\\alpha&H"+ALPHA(obj.fadein)+"\\move("+INT2STR(obj.x0)+","+INT2STR(obj.y0)+","+INT2STR(obj.x1)+","+INT2STR(obj.y1)+")}"+content_correction+"\n";
    return movestr;
}
inline long FixRY(long int x,long int lx,long int y) {//What's The HELL!
    x=10; //Actually Do Nothing
    lx=10;//Actually Do Nothing
    while(y<-180)y+=360;
    while(y>180)y-=360;
    long int t1=0;
    if(y<=90&&y>=-90) {t1=abs(y);} else {t1=180-abs(y);};
    long int delta=floor(t1*2/3);//need improve
    if((y>0&&y<=90)||y<=-90){y-=delta;} else {y+=delta;}
    return y;
}
string AssCompressor::ACC_STR(Comment_struct &obj) {
    string movestr="";
    string fadestr="\\fade("+INT2STR(obj.fadein)+","+INT2STR(obj.fadeout)+",0,0,"+INT2STR(floor(obj.duration*1000))+","+INT2STR(floor(obj.duration*1000*2))+","+INT2STR(floor(obj.duration*1000*3))+")";
    long int xx=0,xx2=0, yy=0, yy2=0;
    string org_command="";
    if (config.ACCFixRotate) {
        long int Fix_Rotate_Y=FixRY(obj.x0, config.DisplayX/2, obj.y_rotate);
        long double zr=Fix_Rotate_Y*M_PI/(double)180;
        long int k=(1-obj.y0/config.DisplayY)*obj.font_size;
        long int ox=obj.x0+k*sin(zr), oy=obj.y0+k*cos(zr);
        xx=ox, yy=oy-k;
        long int Fix_Rotate_Y_1=FixRY(obj.x1, config.DisplayX/2, obj.y_rotate);
        long double zr2=Fix_Rotate_Y_1*M_PI/(double)180;
        long int k2=(1-obj.y1/config.DisplayY)*obj.font_size;
        long int ox1=obj.x1+k2*sin(zr2), oy2=obj.y1+k2*cos(zr2);
        xx2=ox1, yy2=oy2-k2;
        org_command="\\org("+INT2STR(ox)+","+INT2STR(oy)+")";
    } else {
        xx=obj.x0, yy=obj.y0;
        xx2=obj.x1,yy2=obj.y1;
    }
    movestr="Dialogue: "+INT2STR(obj.layer)+","+ASS_TIME(obj.time)+","+ASS_TIME(obj.timeout)+",BufferFront,,0000,0000,0000,,{"+obj.archer+BorderSelector(obj)+"\\fs"+INT2STR(obj.font_size)+"\\c&H"+HEX_COLOR(obj.color)+"\\move("+INT2STR(xx)+","+INT2STR(yy)+","+INT2STR(xx2)+","+INT2STR(yy2)+","+INT2STR(obj.Moving_Pause)+","+INT2STR(obj.Moving_Duration)+")"+"\\frz"+INT2STR(obj.z_rotate)+"\\fry"+INT2STR(obj.y_rotate)+"\\fn"+obj.fontname+fadestr+org_command+"}"+obj.content+"\n";
    return movestr;
}
void AssCompressor::InjectHeader() {
    ASS="[Script Info]\n";
    ASS+=";Build By BiliBuffer_AssCompressor\n";
    ASS+=";From Comment XML URL:"+config.XMLUrl+"\n";
    ASS+=";AssCompressor -- Open Source XML2ASS\n";
    ASS+=";GitHub:https://github.com/CHOSX/AssCompressor.git  [Git]\n";
    ASS+=";GitHub:https://github.com/CHOSX/AssCompressor      [Subversion]\n";
    ASS+="ScriptType: v4.00+\n";
    ASS+="Collisions: Normal\n";
    ASS+="PlayResX: "+INT2STR(config.DisplayX)+"\n";
    ASS+="PlayResY: "+INT2STR(config.DisplayY)+"\n\n";
    ASS+="[V4+ Styles]\n";
    ASS+="Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ";
    ASS+="ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n";
    ASS+="Style: Default, "+config.DefaultFontName+",54,&H00FFFFFF,&H00FFFFFF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,1,0,2,20,20,20,0\n";
    ASS+="Style: BufferFront, "+config.DefaultFontName+",32,&H00FFFFFF,&H00FFFFFF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,1,0,2,20,20,20,0\n\n";
    ASS+="[Events]\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n";
}
void AssCompressor::doConvert_Bilibili_XML(){
    InjectHeader();
    BilibiliCommentManager BilibiliObject(config);
    BilibiliObject.Analysis();
    vector<Comment_struct> &ASS_OBJ=BilibiliObject.FetchData();
    long int OBJ_LEN=ASS_OBJ.size();
    for (int i=0; i<OBJ_LEN; i++)
        if ((ASS_OBJ[i].type==Bilibili_TOP)or(ASS_OBJ[i].type==Bilibili_BOTTOM))
            ASS+=POS_STR(ASS_OBJ[i]);
        else if (ASS_OBJ[i].type==Bilibili_ROLL)
            ASS+=MOVE_STR(ASS_OBJ[i]);
        else if (ASS_OBJ[i].type==Bilibili_ACC)
            ASS+=ACC_STR(ASS_OBJ[i]);
        else
            ASS+=";This CommentType Not Supported\n";
            
	ASS+="\n;Total Comment Analysis: "+INT2STR(ASS_OBJ.size());
	if (BilibiliObject.Warning_Message.size()>0)
        ASS+="\n;Warning Message:\n";
    for (unsigned int i=0; i<BilibiliObject.Warning_Message.size(); i++) {
        ASS+=";"+BilibiliObject.Warning_Message[i]+"\n";
    }
    ASS+="\n"+ASS_Write_Config();
    if (config.ExportLocation!="") {
        ofstream Ass_File(config.ExportLocation.c_str());
        Ass_File<<ASS;
        Ass_File.close();
    }
}

inline string AssCompressor::Colorize_Bool(bool bool_val){
    return (bool_val)?"True":"False";
}

inline bool AssCompressor::num_string(string& str){
    for (size_t i=0; i<str.length(); i++) {
        if ((str[i]<'0')or(str[i]>'9')){return false;}
    }
    return true;
}

inline string AssCompressor::INT2STR(long value){
    unsigned int long_length=0;
    unsigned long test_long=numeric_limits<long>::max();
    while (test_long!=0) {
        long_length++;
        test_long=test_long/10;
    }
    char* buffer_array=(char*)malloc(sizeof(char)*(long_length+5));
    sprintf(buffer_array, "%ld", value);
    string return_str=buffer_array;
    free(buffer_array);
    return return_str;
}

inline string AssCompressor::ASS_Write_Config(){
    #define ASS_Write_Config_Data_Append(item,data)     config_str+=";"+string(item)+"\t"+data+"\n"
    #define ASS_Write_Config_Bool(bol)                  ((bol)?"True":"False")
    string config_str=";======[Configure]================================================\n";
    ASS_Write_Config_Data_Append("ACCEnable\t", ASS_Write_Config_Bool(config.ACCEnable));
    ASS_Write_Config_Data_Append("TOPEnable\t", ASS_Write_Config_Bool(config.TOPEnable));
    ASS_Write_Config_Data_Append("BOTEnable\t", ASS_Write_Config_Bool(config.BOTEnable));
    ASS_Write_Config_Data_Append("ROLEnable\t", ASS_Write_Config_Bool(config.ROLEnable));
    ASS_Write_Config_Data_Append("AlphaACC\t", INT2STR(static_cast<long>(config.AlphaACC*100)));
    ASS_Write_Config_Data_Append("AlphaTOP\t", INT2STR(static_cast<long>(config.AlphaTOP*100)));
    ASS_Write_Config_Data_Append("AlphaBOT\t", INT2STR(static_cast<long>(config.AlphaBOT*100)));
    ASS_Write_Config_Data_Append("AlphaROL\t", INT2STR(static_cast<long>(config.AlphaROL*100)));
    ASS_Write_Config_Data_Append("LayerACC\t", INT2STR(config.layerACC));
    ASS_Write_Config_Data_Append("LayerTOP\t", INT2STR(config.layerTOP));
    ASS_Write_Config_Data_Append("LayerBOT\t", INT2STR(config.layerBOT));
    ASS_Write_Config_Data_Append("LayerROL\t", INT2STR(config.layerROL));
    ASS_Write_Config_Data_Append("DisplayX\t", INT2STR(config.DisplayX));
    ASS_Write_Config_Data_Append("DispalyY\t", INT2STR(config.DisplayY));
    ASS_Write_Config_Data_Append("FontNameHei\t", config.FontNameHei);
    ASS_Write_Config_Data_Append("FontNameYouYuan", config.FontNameYouYuan);
    ASS_Write_Config_Data_Append("FontNameSong\t", config.FontNameSong);
    ASS_Write_Config_Data_Append("FontNameKai\t", config.FontNameKai);
    ASS_Write_Config_Data_Append("FontNameMSHei\t", config.FontNameMSHei);
    ASS_Write_Config_Data_Append("MultilineComment", ASS_Write_Config_Bool(config.MultilineComment));
    ASS_Write_Config_Data_Append("MultilineCompare", ASS_Write_Config_Bool(config.MultilineCompare));
    ASS_Write_Config_Data_Append("FollowACCFontName", ASS_Write_Config_Bool(config.FollowACCFontName));
    ASS_Write_Config_Data_Append("Bold\t\t", ASS_Write_Config_Bool(config.Bold));
    ASS_Write_Config_Data_Append("ACCAutoAdjust\t", ASS_Write_Config_Bool(config.ACCAutoAdjust));
    ASS_Write_Config_Data_Append("ACCFixRotate\t", ASS_Write_Config_Bool(config.ACCFixRotate));
    ASS_Write_Config_Data_Append("MovingVelocity\t", INT2STR(static_cast<long>(config.MovingVelocity*100)));
    ASS_Write_Config_Data_Append("DynamicLengthRatio", INT2STR(static_cast<long>(config.DynamicLengthRatio*100)));
    ASS_Write_Config_Data_Append("DefaultFontSize", INT2STR(config.DefaultFontSize));
    ASS_Write_Config_Data_Append("DefaultFontName", config.DefaultFontName);
    ASS_Write_Config_Data_Append("VideoRatio\t", config.VideoRatio);
    ASS_Write_Config_Data_Append("AllowVisitorComment", ASS_Write_Config_Bool(config.Allow_VisitorComment));
    ASS_Write_Config_Data_Append("comment_at_timestamp", ((config.comment_at_timestamp==BILIBILI_NO_SPECIFIC_TIMESTAMP)?"N/A":INT2STR(config.comment_at_timestamp)));
    ASS_Write_Config_Data_Append("RemoveColor\t", ((config.remove_color.size()==0)?"N/A":"True"));
    for (size_t i=0; i<config.remove_color.size(); i++) {
        ASS_Write_Config_Data_Append("           \t", config.remove_color[i]);
    }
    ASS_Write_Config_Data_Append("RemoveUser\t", ((config.remove_user.size()==0)?"N/A":"True"));
    for (size_t i=0; i<config.remove_user.size(); i++) {
        ASS_Write_Config_Data_Append("           \t", config.remove_user[i]);
    }
    ASS_Write_Config_Data_Append("RemoveKey\t", ((config.remove_key.size()==0)?"N/A":"True"));
    for (size_t i=0; i<config.remove_key.size(); i++) {
        ASS_Write_Config_Data_Append("           \t", config.remove_key[i]);
    }
    ASS_Write_Config_Data_Append("RemovePool\t", ((config.remove_pool.size()==0)?"N/A":"True"));
    for (size_t i=0; i<config.remove_pool.size(); i++) {
        ASS_Write_Config_Data_Append("           \t", config.remove_pool[i]);
    }
    config_str+=";======[Configure]================================================\n";
    return config_str;
}