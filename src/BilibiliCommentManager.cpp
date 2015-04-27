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
#include "BilibiliCommentManager.h"

BilibiliCommentManager::BilibiliCommentManager(AssConfig &conf):configure(conf) {
    //comment_xml_content=configure.XMLContent;
    FixBilibiliCommentXML();
    
	for (int i=0; i<=configure.DisplayY; i++) {
        //Init the Dynamic Compare Array
        Dynamic_ROLL.push_back(0.00);
        Dynamic_BOTTOM.push_back(0.00);
        Dynamic_TOP.push_back(0.00);
    }
}
void BilibiliCommentManager::FixBilibiliCommentXML(){
    vector<string> Comment_Array;
    StringSplit(comment_xml_content, Bilibili_Comment_Line_End,Comment_Array);
    string Corrected_XML="";
    for (unsigned int i=0; i<Comment_Array.size(); i++)
        if (Comment_Array[i].find(Bilibili_Comment_Error)==Comment_Array[i].npos)
            Corrected_XML+=Comment_Array[i]+Bilibili_Comment_Line_End;
        else
            Warning_Message.push_back(Bilibili_Warning_Init_Error+Comment_Array[i]);
    Corrected_XML+="</i>";
    comment_xml_content=Corrected_XML;
}
void BilibiliCommentManager::Font_Correction(Comment_struct &obj){
    if (obj.fontname==BilibiliFont_KEY_HEI)
        obj.fontname=configure.FontNameHei;
    else if (obj.fontname==BilibiliFont_KEY_Kai)
        obj.fontname=configure.FontNameKai;
    else if (obj.fontname==BilibiliFont_KEY_MS_HEI)
        obj.fontname=configure.FontNameMSHei;
    else if (obj.fontname==BilibiliFont_KEY_Song)
        obj.fontname=configure.FontNameSong;
    else if (obj.fontname==BilibiliFont_KEY_YouYuan)
        obj.fontname=configure.FontNameYouYuan;
    else if (obj.fontname!=configure.DefaultFontName)
        Warning_Message.push_back(Bilibili_Warning_Font_Error+obj.fontname);
}

#define STR2INT(str) (atoi((str).c_str()))
#define STR2DOUBLE(str) (atof((str).c_str()))
double BilibiliCommentManager::surval_time() {
    return (double)Bilibili_Survival_Time*(1/configure.MovingVelocity)*(configure.DisplayX/(double)configure.iframeX);
}
string BilibiliCommentManager::destr(string &input) {
    return input.substr(1,input.length()-2);
}
/*bool _Comment_Request_Compare(const Comment_struct &obj_a,const Comment_struct &obj_b) {
    return obj_a.time<obj_b.time;
}*/

bool operator < (Comment_struct s1, Comment_struct s2){return s1.time<s2.time;}

void BilibiliCommentManager::SortRequest(vector<Comment_struct> &Array){
    stable_sort(Array.begin(), Array.end(), less<Comment_struct>());
}

vector<Comment_struct> &BilibiliCommentManager::FetchData() {return Comment_Request;}

long int BilibiliCommentManager::FetchNumComment() {return total_comment;}

inline long int BilibiliCommentManager::HeightOccopuy(Comment_struct &object) {
    return configure.MultilineCompare?object.font_size*(object.enter+1):object.font_size;
}
vector<string> BilibiliCommentManager::StringSplit(string s,char c){
    vector<string> v;
    StringSplit(s,c,v);
    return v;
}
void BilibiliCommentManager::StringSplit(string content, char seek,vector<string> &strvec) {
    strvec.clear();
    string::size_type pos1,pos2;
    pos2=content.find(seek);
    pos1=0;
    while (string::npos != pos2) {
        strvec.push_back(content.substr(pos1,pos2-pos1));
        pos1=pos2+1;
        pos2=content.find(seek,pos1);
    }
    strvec.push_back(content.substr(pos1));
}
void BilibiliCommentManager::Comment_Analysis_Single_Normal(string attribute, string content,Comment_struct &returnvalue) {
    vector<string> attribute_array;
    StringSplit(attribute, ',',attribute_array);
    returnvalue.time=STR2DOUBLE(attribute_array[0]);
    returnvalue.type=STR2INT(attribute_array[1]);
    returnvalue.font_size=configure.DefaultFontSize*(STR2INT(attribute_array[2]))/25;
    if ((returnvalue.type==Bilibili_ACC)and(configure.VideoRatio=="16:9")) {
        double ratio=configure.DisplayX/(double)configure.iframeX;
        returnvalue.font_size=floor(STR2INT(attribute_array[2])*ratio);
    }
    if ((returnvalue.type==Bilibili_ACC)and(configure.VideoRatio=="4:3")) {
        double ratio=configure.DisplayY/(double)configure.iframeY;
        returnvalue.font_size=floor(STR2INT(attribute_array[2])*ratio);
    }
    returnvalue.color=STR2INT(attribute_array[3]);
    returnvalue.timestamp=STR2INT(attribute_array[4]);
    returnvalue.pool=STR2INT(attribute_array[5]);
    returnvalue.user_id=attribute_array[6];
    returnvalue.comment_id=attribute_array[7];
    returnvalue.content=content;
    if ((returnvalue.type==Bilibili_TOP)or(returnvalue.type==Bilibili_BOTTOM)){
        returnvalue.leftin=returnvalue.time;
        returnvalue.rightin=returnvalue.time;
        returnvalue.leftout=returnvalue.time;
        returnvalue.rightout=returnvalue.time+Bilibili_Survival_Time*(1/configure.MovingVelocity);
    }
    if (returnvalue.type==Bilibili_ROLL){
        returnvalue.leftin=returnvalue.time;
        long int DynamicX=floor(configure.DisplayX*configure.DynamicLengthRatio);
        double velocity=((configure.DisplayX+returnvalue.content.length()*returnvalue.font_size)/surval_time());
        returnvalue.leftout=DynamicX/velocity+returnvalue.time;
        returnvalue.rightin=(returnvalue.content.length()*returnvalue.font_size)/velocity+returnvalue.time;
        returnvalue.rightout=(DynamicX+returnvalue.content.length()*returnvalue.font_size)/velocity+returnvalue.time;
    }
    returnvalue.enter=0;
    if (configure.MultilineComment){
        string input_str=returnvalue.content;
        const string search_str="/n";
        const string replace_str="\\N";
        long int pos=input_str.find(search_str);
        returnvalue.max_length_line=pos;
        long int init_pos=pos;
        while (pos!=-1){
            input_str.replace(pos,search_str.length(),replace_str);
            pos=input_str.find(search_str);
            returnvalue.enter++;
            if (pos!=-1) {
                long int temp_compare=pos-init_pos;
                if (temp_compare>returnvalue.max_length_line) {
                    returnvalue.max_length_line=temp_compare;
                }
            }
        }
        returnvalue.content=input_str;
    }
}
void BilibiliCommentManager::Comment_Analysis_Single_Normal_ACC(Comment_struct &obj) {
    vector<string> temp_array;
    StringSplit(destr(obj.content),',',temp_array);
    for (unsigned int i=0; i<temp_array.size(); i++) {
        if (temp_array[i][0]=='"') {
            temp_array[i]=destr(temp_array[i]);
        }
    }
    obj.layer=configure.layerACC;
    double fadein=STR2DOUBLE(StringSplit(temp_array[2], '-')[0]);
    double fadeout=STR2DOUBLE(StringSplit(temp_array[2], '-')[1]);
    double duration=STR2DOUBLE(temp_array[3]);
    //Comment_struct before,moving,after;
    //before=obj; moving=obj; after=obj;
    Comment_struct ACC_Comment=obj;
    if (temp_array.size()>14) {
        //Trace-Line Not Supported Yet
        //[x0,y0,”fadein-fadeout”,duration(seconds),”content”,z-rotate,y-rotate,x1,y1,moving-duration(ms),moving-pause(ms),highlight,”front name”,linear-movement,”trace-line”] 15
        Warning_Message.push_back(Bilibili_Warning_ACC15+obj.content);
        return;
    }
    if (temp_array.size()==14) {
        //[“X[0]”,”Y[1]",”衰弱透明度[2]",”生存时间[3]",”弹幕内容[4]","z轴旋转[5]","y轴旋转[6]","x to[7]","y to[8]","移动耗时[9]",”移动暂停[10]”,”Hightlight”,"字体","线性移动”]  14
        ACC_Comment.type=Bilibili_ACC;
        ACC_Comment.archer="\\an7";
        ACC_Comment.duration=duration;
        ACC_Comment.Moving_Pause=STR2INT(temp_array[10]);
        ACC_Comment.Moving_Duration=STR2INT(temp_array[9]);
        ACC_Comment.fadein=fadein;
        ACC_Comment.fadeout=fadeout;
        ACC_Comment.z_rotate=(-1)*STR2INT(temp_array[5]);
        ACC_Comment.y_rotate=(-1)*STR2INT(temp_array[6]);
        double x0=STR2DOUBLE(temp_array[0]);
        double y0=STR2DOUBLE(temp_array[1]);
        double x1=STR2DOUBLE(temp_array[7]);
        double y1=STR2DOUBLE(temp_array[8]);
        if (x0<1) {x0=x0*configure.DisplayX;}
        if (x1<1) {x1=x1*configure.DisplayX;}
        if (y0<1) {y0=y0*configure.DisplayY;}
        if (y1<1) {y1=y1*configure.DisplayY;}
        ACC_Comment.x0=floor(x0); ACC_Comment.x1=floor(x1);
        ACC_Comment.y0=floor(y0); ACC_Comment.y1=floor(y1);
        ACC_Comment.content=temp_array[4];
        if (configure.FollowACCFontName) {
            ACC_Comment.fontname=temp_array[12];
        }else{
            ACC_Comment.fontname=configure.DefaultFontName;
        }
        ACC_Comment.timeout=ACC_Comment.time+ACC_Comment.duration;
        total_comment++;
        Comment_Analysis_Single_Further_ACC(ACC_Comment);
        Comment_Request.push_back(ACC_Comment);
    }
    if (temp_array.size()==13) {   //ACC-13 Format
        //["X","Y","衰弱透明度","生存时间","弹幕内容","z轴旋转","y轴旋转","x to","y to","移动耗时","移动暂停","字体","线性移动”]  13
        ACC_Comment.type=Bilibili_ACC;
        ACC_Comment.archer="\\an7";
        ACC_Comment.duration=duration;
        ACC_Comment.Moving_Pause=STR2INT(temp_array[10]);
        ACC_Comment.Moving_Duration=STR2INT(temp_array[9]);
        ACC_Comment.fadein=fadein;
        ACC_Comment.fadeout=fadeout;
        ACC_Comment.z_rotate=(-1)*STR2INT(temp_array[5]);
        ACC_Comment.y_rotate=(-1)*STR2INT(temp_array[6]);
        double x0=STR2DOUBLE(temp_array[0]);
        double y0=STR2DOUBLE(temp_array[1]);
        double x1=STR2DOUBLE(temp_array[7]);
        double y1=STR2DOUBLE(temp_array[8]);
        if (x0<1) {x0=x0*configure.DisplayX;}
        if (x1<1) {x1=x1*configure.DisplayX;}
        if (y0<1) {y0=y0*configure.DisplayY;}
        if (y1<1) {y1=y1*configure.DisplayY;}
        ACC_Comment.x0=floor(x0);    ACC_Comment.x1=floor(x1);
        ACC_Comment.y0=floor(y0);    ACC_Comment.y1=floor(y1);
        ACC_Comment.content=temp_array[4];
        if (configure.FollowACCFontName) {
            ACC_Comment.fontname=temp_array[11];
        }else{
            ACC_Comment.fontname=configure.DefaultFontName;
        }
        ACC_Comment.timeout=ACC_Comment.time+ACC_Comment.duration;
        total_comment++;
        Comment_Analysis_Single_Further_ACC(ACC_Comment);
        Comment_Request.push_back(ACC_Comment);
    }
    if (temp_array.size()==11) {    //ACC-11 Format
        //[“X”0,”Y"1,"衰弱透明度”2,"生存时间”3,"弹幕内容”4,"z轴旋转”5,"y轴旋转”6,"x to”7,"y to”8,"移动耗时”9,"移动暂停”10]  11
        ACC_Comment.type=Bilibili_ACC;
        ACC_Comment.archer="\\an7";
        ACC_Comment.duration=ACC_Comment.duration;
        ACC_Comment.Moving_Pause=STR2INT(temp_array[10]);
        ACC_Comment.Moving_Duration=STR2INT(temp_array[9]);
        ACC_Comment.fadein=fadein;
        ACC_Comment.fadeout=fadeout;
        ACC_Comment.z_rotate=(-1)*STR2INT(temp_array[5]);
        ACC_Comment.y_rotate=(-1)*STR2INT(temp_array[6]);
        double x0=STR2DOUBLE(temp_array[0]);
        double y0=STR2DOUBLE(temp_array[1]);
        double x1=STR2DOUBLE(temp_array[7]);
        double y1=STR2DOUBLE(temp_array[8]);
        if (x0<1) {x0=x0*configure.DisplayX;}
        if (x1<1) {x1=x1*configure.DisplayX;}
        if (y0<1) {y0=y0*configure.DisplayY;}
        if (y1<1) {y1=y1*configure.DisplayY;}
        ACC_Comment.x0=floor(x0);    ACC_Comment.x1=floor(x1);
        ACC_Comment.y0=floor(y0);    ACC_Comment.y1=floor(y1);
        ACC_Comment.content=temp_array[4];
        ACC_Comment.fontname=configure.DefaultFontName;
        ACC_Comment.timeout=ACC_Comment.time+ACC_Comment.duration;
        total_comment++;
        Comment_Analysis_Single_Further_ACC(ACC_Comment);
        Comment_Request.push_back(ACC_Comment);
    }
    if (temp_array.size()==7) {     //ACC-7 Format
        //[“X”0,”Y”1,"衰弱透明度”2,"生存时间”3,"弹幕内容”4,"z轴旋转”5,"y轴旋转”6]  7
        ACC_Comment.type=Bilibili_ACC;
        ACC_Comment.archer="\\an7";
        ACC_Comment.duration=ACC_Comment.duration;
        ACC_Comment.Moving_Duration=0;
        ACC_Comment.Moving_Pause=0;
        ACC_Comment.fadein=fadein;  ACC_Comment.fadeout=fadeout;
        ACC_Comment.z_rotate=(-1)*STR2INT(temp_array[5]);
        ACC_Comment.y_rotate=(-1)*STR2INT(temp_array[6]);
        double x0=STR2DOUBLE(temp_array[0]);
        double y0=STR2DOUBLE(temp_array[1]);
        if (x0<1) {x0=x0*configure.DisplayX;}
        if (y0<1) {y0=y0*configure.DisplayY;}
        ACC_Comment.x0=floor(x0);    ACC_Comment.x1=floor(x0);
        ACC_Comment.y0=floor(y0);    ACC_Comment.y1=floor(y0);
        ACC_Comment.content=temp_array[4];
        ACC_Comment.fontname=configure.DefaultFontName;
        ACC_Comment.timeout=ACC_Comment.time+ACC_Comment.duration;
        total_comment++;
        Comment_Analysis_Single_Further_ACC(ACC_Comment);
        Comment_Request.push_back(ACC_Comment);
    }
    if (temp_array.size()==5) {     //ACC-5 Format
        //[“X"0,”Y"1,"衰弱透明度”2,"生存时间”3,"弹幕内容”4]  5
        ACC_Comment.type=Bilibili_ACC;
        ACC_Comment.duration=ACC_Comment.duration;
        ACC_Comment.Moving_Duration=0;
        ACC_Comment.Moving_Pause=0;
        ACC_Comment.fadein=fadein;  ACC_Comment.fadeout=fadeout;
        ACC_Comment.z_rotate=0;     ACC_Comment.y_rotate=0;
        double x0=STR2DOUBLE(temp_array[0]);
        double y0=STR2DOUBLE(temp_array[1]);
        if (x0<1) {x0=x0*configure.DisplayX;}
        if (y0<1) {y0=y0*configure.DisplayY;}
        ACC_Comment.x0=floor(x0);    ACC_Comment.x1=floor(x0);
        ACC_Comment.y0=floor(y0);    ACC_Comment.y1=floor(y0);
        ACC_Comment.content=temp_array[4];
        ACC_Comment.fontname=configure.DefaultFontName;
        ACC_Comment.timeout=ACC_Comment.time+ACC_Comment.duration;
        total_comment++;
        Comment_Analysis_Single_Further_ACC(ACC_Comment);
        Comment_Request.push_back(ACC_Comment);
    }
}
void BilibiliCommentManager::Analysis() {
    using namespace rapidxml;
    char* str=(char*)malloc(configure.XMLContent.size()+1);
    strcpy(str,configure.XMLContent.c_str());
    xml_document<> doc;
    doc.parse<0>(str);
    xml_node<> *root=doc.first_node();
    xml_node<> *commentElement=root->first_node("d");
    //Initial Analysis
    for (; commentElement!=NULL; commentElement=commentElement->next_sibling()) {
        xml_attribute<> *pAttribute=commentElement->first_attribute("p");
        string commentContent=commentElement->value();
        if (commentContent=="")continue;
        Comment_struct temp_Request;
        if (pAttribute==NULL)continue;
        Comment_Analysis_Single_Normal(pAttribute->value(),commentContent,temp_Request);
        /////////////////////////////////////////////////////////////////////////////////AssCompressor Ver 1.2.0
        bool will_be_removed=false;
        if ((temp_Request.user_id[0]=='D')and(configure.Allow_VisitorComment==false)) {will_be_removed=true;}
        for (size_t i=0; i<configure.remove_pool.size(); i++){
            if (INT2STR(temp_Request.pool)==configure.remove_pool[i]) {
                will_be_removed=true;
            }
        }
        for (size_t i=0; i<configure.remove_user.size(); i++) {
            if (temp_Request.user_id==configure.remove_user[i]) {
                will_be_removed=true;
            }
        }
        for (size_t i=0; i<configure.remove_key.size(); i++) {
            if (temp_Request.content.find(configure.remove_key[i].c_str())!=temp_Request.content.npos) {
                will_be_removed=true;
            }
        }
        for (size_t i=0; i<configure.remove_color.size(); i++) {
            if (INT2STR(temp_Request.color)==configure.remove_color[i]) {
                will_be_removed=true;
            }
            if (HEXColor(temp_Request.color)==configure.remove_color[i]) {
                will_be_removed=true;
            }
        }
        if (will_be_removed) {continue;}
        ////////////////////////////////////////////////////////////////////////////////////////////////////////
        if ((temp_Request.type==Bilibili_ROLL)and(configure.ROLEnable))
            Comment_Request.push_back(temp_Request);
        else if ((temp_Request.type==Bilibili_BOTTOM)and(configure.BOTEnable))
            Comment_Request.push_back(temp_Request);
        else if ((temp_Request.type==Bilibili_TOP)and(configure.TOPEnable))
            Comment_Request.push_back(temp_Request);
        else if ((temp_Request.type==Bilibili_ACC)and(configure.ACCEnable))
            Comment_Analysis_Single_Normal_ACC(temp_Request);
    }
    doc.clear();
    free(str);
    total_comment=Comment_Request.size();
    SortRequest(Comment_Request);
    for (int i=0; i<total_comment; i++) {
        if (Comment_Request[i].type==Bilibili_ROLL)
            Comment_Analysis_Single_Further_ROLL(Comment_Request[i]);
        else if (Comment_Request[i].type==Bilibili_BOTTOM)
            Comment_Analysis_Single_Further_Bottom(Comment_Request[i]);
        else if (Comment_Request[i].type==Bilibili_TOP)
            Comment_Analysis_Single_Further_TOP(Comment_Request[i]);
    }
    //Further Analysis Started
}
long int BilibiliCommentManager::ROLL_Y(Comment_struct &roll) {
    bool flag2;
    for (int i=0; i<=configure.DisplayY-HeightOccopuy(roll); i++) {
        flag2=true;
        for (int j=0; j<HeightOccopuy(roll); j++) {
            if (Dynamic_ROLL[i+j]>roll.leftout) {flag2=false;} //Check for Space
        }
        if (flag2) {//Place Found!
            for (int k=0; k<HeightOccopuy(roll); k++) {     //Refresh
                Dynamic_ROLL[i+k]=roll.rightout;
            }
            return i;
        }
    }
    long int returnvalue=0;
    double exceed=LLONG_MAX;
    double temp_exceed=0.00;
    for (int i=0; i<=configure.DisplayY-HeightOccopuy(roll); i++) {
        temp_exceed=0.00;
        for (int j=0; j<HeightOccopuy(roll); j++) {
            if (Dynamic_ROLL[i+j]>roll.leftout) {
                temp_exceed+=Dynamic_ROLL[i+j]-roll.leftout;  //Collect Time Exceed
            }
        }
        if (temp_exceed<exceed) {
            returnvalue=i;
            exceed=temp_exceed;
        }
    }
    for (int i=0; i<HeightOccopuy(roll); i++) {Dynamic_ROLL[returnvalue+i]=roll.rightout;}  //Refresh
    return returnvalue;
}
long int BilibiliCommentManager::TOP_Y(Comment_struct &top) {
    bool flage=true;
    for (int i=0; i<configure.DisplayX-HeightOccopuy(top); i++) {
        flage=true;
        for (int j=0; j<HeightOccopuy(top); j++) {
            if (Dynamic_TOP[i+j]>top.leftout) {
                flage=false;
            }
        }
        if (flage) {
            for (int j=0; j<HeightOccopuy(top); j++) {  //Refresh
                Dynamic_TOP[i+j]=top.rightout;
            }
            return i;
        }
    }
    long int returnvalue=0;
    double exceed=LLONG_MAX;
    double temp_exceed=0.00;
    for (int i=0; i<configure.DisplayX-HeightOccopuy(top); i++) {
        temp_exceed=0.00;
        for (int j=0; j<HeightOccopuy(top); j++) {
            if (Dynamic_TOP[i+j]>top.leftout) {
                temp_exceed+=Dynamic_TOP[i+j]-top.leftout;
            }
        }
        if (temp_exceed<exceed) {
            exceed=temp_exceed;
            returnvalue=i;
        }
    }
    for (int i=0; i<HeightOccopuy(top); i++) {Dynamic_TOP[returnvalue+i]=top.rightout;}
    return returnvalue;
}
long int BilibiliCommentManager::BOTTOM_Y(Comment_struct &bottom) {
    bool flage=true;
    for (long int i=configure.DisplayY-HeightOccopuy(bottom); i>=0; i--) {
        flage=true;
        for (long int j=0; j<HeightOccopuy(bottom); j++) {
            if (Dynamic_BOTTOM[i+j]>bottom.leftout) {
                flage=false;
            }
        }
        if (flage) {    //Found
            for (long int k=0; k<HeightOccopuy(bottom); k++) {  //Refresh
                Dynamic_BOTTOM[i+k]=bottom.rightout;
            }
            return i;
        }
    }
    long int returnvalue=0;
    double exceed=LLONG_MAX;
    double temp_exceed=0.00;
    for (long int i=configure.DisplayY-HeightOccopuy(bottom); i>=0; i--) {
        temp_exceed=0.00;
        for (long int j=0; j<HeightOccopuy(bottom); j++) {
            if (Dynamic_BOTTOM[i+j]>bottom.leftout) {
                temp_exceed+=Dynamic_BOTTOM[i+j]-bottom.leftout;
            }
        }
        if (temp_exceed<exceed) {
            exceed=temp_exceed;
            returnvalue=i;
        }
    }
    for (long int i=0; i<HeightOccopuy(bottom); i++) {Dynamic_BOTTOM[returnvalue+i]=bottom.rightout;}
    return returnvalue;
}
void BilibiliCommentManager::Comment_Analysis_Single_Further_ROLL(Comment_struct &ROLL) {
    ROLL.archer="\\an7";
    ROLL.x0=configure.DisplayX; ROLL.x1=-1*ROLL.content.length()*ROLL.font_size;
    ROLL.fadein=255-floor(255*configure.AlphaROL);
    ROLL.fadeout=255-floor(255*configure.AlphaROL);
    //double velocity=102.4*2;   //Maintaining Velocity
    ROLL.duration=/*(configure.DisplayX+ROLL.content.length()*ROLL.font_size)/velocity;*/surval_time();
    ROLL.z_rotate=0; ROLL.y_rotate=0;
    ROLL.linear_acceleration=false;
    ROLL.fontname=configure.DefaultFontName;
    ROLL.border_enhance=true;
    ROLL.y0=ROLL_Y(ROLL);
    ROLL.y1=ROLL.y0;
    ROLL.timeout=ROLL.time+ROLL.duration;
    ROLL.layer=configure.layerROL;
}
void BilibiliCommentManager::Comment_Analysis_Single_Further_TOP(Comment_struct &TOP) {
    TOP.archer="\\an8";
    TOP.x0=configure.DisplayX/2;TOP.x1=configure.DisplayX/2;
    TOP.fadein=255-floor(255*configure.AlphaTOP);
    TOP.fadeout=255-floor(255*configure.AlphaTOP);
    TOP.duration=Bilibili_Survival_Time*(1/configure.MovingVelocity);
    TOP.z_rotate=0;TOP.y_rotate=0;
    TOP.linear_acceleration=false;
    TOP.fontname=configure.DefaultFontName;
    TOP.border_enhance=true;
    TOP.y0=TOP_Y(TOP);TOP.y1=TOP.y0;
    TOP.timeout=TOP.time+TOP.duration;
    TOP.layer=configure.layerTOP;
}
void BilibiliCommentManager::Comment_Analysis_Single_Further_Bottom(Comment_struct &BOTTOM) {
    BOTTOM.archer="\\an8";
    BOTTOM.x0=configure.DisplayX/2; BOTTOM.x1=configure.DisplayX/2;
    BOTTOM.fadein=255-floor(255*configure.AlphaBOT);
    BOTTOM.fadeout=255-floor(255*configure.AlphaBOT);
    BOTTOM.duration=Bilibili_Survival_Time*(1/configure.MovingVelocity);
    BOTTOM.z_rotate=0;  BOTTOM.y_rotate=0;
    BOTTOM.linear_acceleration=false;
    BOTTOM.fontname=configure.DefaultFontName;
    BOTTOM.border_enhance=true;
    BOTTOM.y0=BOTTOM_Y(BOTTOM); BOTTOM.y1=BOTTOM.y0;
    BOTTOM.timeout=BOTTOM.time+BOTTOM.duration;
    BOTTOM.layer=configure.layerTOP;
}
void BilibiliCommentManager::Comment_Analysis_Single_Further_ACC(Comment_struct &ACC) {
    //Alpha Value Adjust
    ACC.fadein=255-floor(ACC.fadein*255);
    ACC.fadeout=255-floor(ACC.fadeout*255);   //Change Alpha Value From 1-0 to 0-255
    Font_Correction(ACC);
    //Co-ordinate Need to be Corrected Here
    /*
     这段代码主要是在矫正精确定位弹幕的初始坐标和结束坐标
     因为在bilibili上是按照iframe播放器的大小决定的、然后iframe播放器似乎有dts矫正的功能、
     所以说应该视频正常的状态只有16:9和4:3
     bilibili的自动矫正功能似乎是判断边长的大小来判定的、会尽量趋近于最合适的dts矫正
     所以说目前就按找这个来矫正吧、
     ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
     矫正的原理其实就是计算边上的黑边然后去掉黑边所占的像素
     这里黑边所占的像素被记作balck_margin
     完成这个计算之后在按照伸拉比例放大然后就可以了
     感觉这个应该不是最完美的解决方案、、但是目前测试下来、、这可能是我想到的对快速、、最精确的还原方法了
     以后可能会在这里加上auto的能力、并且解除对于伸拉比例的限制（目前严格只能是16:9,或者是4:3）
     如果不是16:9或者4:3的话程序应该会不会报错、但是结果就会很难看
                                                                  --By CHOSX_K9*/
    if (configure.VideoRatio=="16:9") {
        double front_ratio=configure.DisplayX/(double)configure.iframeX;
        ACC.x0=floor(ACC.x0*front_ratio);
        ACC.x1=floor(ACC.x1*front_ratio);
        double black_margin=floor(configure.iframeY-(configure.iframeX/(double)16)*9)/2;
        ACC.y0=floor(front_ratio*(ACC.y0-black_margin));
        ACC.y1=floor(front_ratio*(ACC.y1-black_margin));
        //Warning Message Will be added
    }else if (configure.VideoRatio=="4:3"){
        double front_ratio=configure.DisplayY/(double)configure.iframeY;
        ACC.y0=floor(ACC.y0*front_ratio);
        ACC.y1=floor(ACC.y1*front_ratio);
        double black_margin=floor(configure.iframeX-(configure.iframeY/(double)3*4))/2;
        ACC.x0=floor(front_ratio*(ACC.x0-black_margin));
        ACC.x1=floor(front_ratio*(ACC.x1-black_margin));
    }else {
        Warning_Message.push_back(Bilibili_Warning_Ratio_Error+ACC.content);
    }
    if (configure.ACCAutoAdjust) {
        Comment_Analysis_Correction_ACC(ACC);
    }
}
void BilibiliCommentManager::Comment_Analysis_Correction_ACC(Comment_struct &obj) {
    if (obj.x0<0) {obj.x0=0;}
    if (obj.x1<0) {obj.x1=0;}
    if (obj.y0<0) {obj.y0=0;}
    if (obj.y1<0) {obj.y1=0;}
    if ((obj.y0+obj.enter*(obj.font_size))>=configure.DisplayY) {
        obj.y0=configure.DisplayY-((obj.enter+1)*obj.font_size);
    }
    if ((obj.y1+obj.enter*(obj.font_size))>=configure.DisplayY) {
        obj.y1=configure.DisplayY-((obj.enter+1)*obj.font_size);
    }
}

inline string BilibiliCommentManager::HEXColor(long input){
    char color[7];
    sprintf(color, "%06lx",input);
    string return_str="      ";
    return_str[0]=color[4];return_str[1]=color[5];
    return_str[2]=color[2];return_str[3]=color[3];
    return_str[4]=color[0];return_str[5]=color[1];
    return return_str;
}

inline string BilibiliCommentManager::INT2STR(long val){
    ostringstream out;
    out<<val;
    return out.str();
}