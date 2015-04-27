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

#include <curl/curl.h>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include "AssCompressor.h"
#include "rapidxml/RapidXML.h"
#include "rapidjson/document.h"

#ifdef _withMultiThread
    #include <thread>
    #include <mutex>
    vector<thread> worker_list;
    mutex thread_lock;
    #define _withMultiThread_Lock       thread_lock.lock();
    #define _withMultiThread_Unlock     thread_lock.unlock();
#else
    #define _withMultiThread_Lock       /*NOTHING NEED HERE*/;
    #define _withMultiThread_Unlock     /*NOTHING NEED HERE*/;
#endif

#ifndef _WIN32_Font
    #define BilibiliFont_Name_HEI      "黑体-简 细体"
    #define BilibiliFont_Name_YouYuan  "圆体-简 细体"
    #define BilibiliFont_Name_Song     "宋体-简 常规体"
    #define BilibiliFont_Name_Kai      "楷体-简 常规体"
    #define BilibiliFont_Name_MS_HEI   "兰亭黑-简 纤黑"
#else
    #define BilibiliFont_Name_HEI      "黑体"
    #define BilibiliFont_Name_YouYuan  "幼圆"
    #define BilibiliFont_Name_Song     "宋体"
    #define BilibiliFont_Name_Kai      "楷体"
    #define BilibiliFont_Name_MS_HEI   "微软雅黑"
#endif

#ifdef _withMultiThread
    #define _flags_withMultiThread      GreenOutput(string("True"))
#else
    #define _flags_withMultiThread      RedOutput(string("False"))
#endif

#ifdef _WIN32_Font
    #define _flags_WIN32_Font           GreenOutput(string("True"))
#else
    #define _flags_WIN32_Font           RedOutput(string("False"))
#endif

#ifdef _withColorOutput
    #define _flags_withColorOutput      GreenOutput(string("True"))
#else
    #define _flags_withColorOutput      RedOutput(string("False"))
#endif

#ifdef _withColorOutput
    #include "colorize.h"
    #define GreenOutput(str)            (Colorize(str,Green))
    #define RedOutput(str)              (Colorize(str,Red))
    #define YellowOutput(str)           (Colorize(str,Yellow))
#else
    #define GreenOutput(str)            (str)
    #define RedOutput(str)              (str)
    #define YellowOutput(str)           (str)
#endif

using namespace std;
using namespace rapidxml;
using namespace rapidjson;

bool flags_version=false;
bool flags_printxmlconfig=false;
bool flags_help=false;
bool flags_compileflags=false;
bool flags_printbuildflags=false;
vector<string> task_list;

inline string INT2STR(long);

void print_copyright(){
    cout<<"AssCompressor Ver."<<AssCompressorVersion<<endl;
    cout<<"Copyright (C) 2014 CHOSX_K9 RecursiveG"<<endl;
    cout<<endl;
    cout<<" This program is free software; you can redistribute it and/or"<<endl;
    cout<<"modify it under the terms of the GNU General Public License"<<endl;
    cout<<"as published by the Free Software Foundation; either version 2"<<endl;
    cout<<"of the License, or (at your option) any later version."<<endl;
    cout<<endl;
    cout<<"This program is distributed in the hope that it will be useful,"<<endl;
    cout<<"but WITHOUT ANY WARRANTY; without even the implied warranty of"<<endl;
    cout<<"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"<<endl;
    cout<<"GNU General Public License for more details."<<endl;
    cout<<endl;
    cout<<"You should have received a copy of the GNU General Public License"<<endl;
    cout<<"along with this program; if not, write to the Free Software"<<endl;
    cout<<"Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA."<<endl;
}

void print_help(string argv0){
    cout<<"AssCompressor Ver."<<AssCompressorVersion<<endl;
    cout<<"Copyright (C) 2014-2015 CHOSX_K9 RecursiveG"<<endl<<endl;
    cout<<"Usage: "<<argv0<<" [Configure XML Path]...[Configure XML Path]"<<endl;
    cout<<"       "<<argv0<<" --version"<<endl;
    cout<<"       "<<argv0<<" --help"<<endl;
    cout<<"       "<<argv0<<" --buildConfig"<<endl;
    cout<<"With Flags:"<<endl;
    cout<<"       --withConfig    Display Config from XML input"<<endl;
}

void print_buildConfig(){
    cout<<"AssCompressor Ver."<<AssCompressorVersion<<endl;
    cout<<"Build At: "<<__DATE__<<" "<<__TIME__<<endl;
    cout<<"Build Configure:"<<endl;
    cout<<"    withColorOutput:      "<<_flags_withColorOutput<<endl;
    cout<<"    withMultiThread:      "<<_flags_withMultiThread<<endl;
    cout<<"    Support WIN32 Font:   "<<_flags_WIN32_Font<<endl;
}

static size_t _onCurlWrite(void* buffer, size_t size, size_t nmemb, string &data)
{
    if(NULL==buffer){return -1;}
    data.append((char*)buffer,size*nmemb);
    return size*nmemb;
}


#define ToBool(str)         ((str)[4]=='\0')    //Assume a four-char-long string as 'true'
                                                //otherwise 'false'

void FillConfigure(AssConfig &conf,const char* cfgPath){
    ifstream in(cfgPath);
    if (!in)throw runtime_error("Corrupted Config: Can't open file: "+string(cfgPath));
    string xml="";char ch;
    while (!in.eof()){
        in.get(ch);
        xml+=ch;
    }
    char str[xml.size()+10];
    strcpy(str,xml.c_str());
    xml_document<> doc;
    doc.parse<0>(str);
    xml_node<> *root=doc.first_node();
    if (strcmp(root->first_attribute("version")->value(),"CLIv1"))
        throw runtime_error("Unsupported Configure version");
        
    conf.ACCEnable=ToBool(root->first_node("ACCEnable")->value());
    conf.TOPEnable=ToBool(root->first_node("TOPEnable")->value());
    conf.BOTEnable=ToBool(root->first_node("BOTEnable")->value());
    conf.ROLEnable=ToBool(root->first_node("ROLEnable")->value());
    conf.FollowACCFontName=ToBool(root->first_node("FollowACCFontName")->value());
    conf.Bold=ToBool(root->first_node("Bold")->value());
    conf.ACCAutoAdjust=ToBool(root->first_node("ACCAutoAdjust")->value());
    conf.ACCFixRotate=ToBool(root->first_node("ACCFixRotate")->value());
    conf.MultilineComment=ToBool(root->first_node("MultilineComment")->value());
    conf.MultilineCompare=conf.MultilineComment&&ToBool(root->first_node("MultilineCompare")->value());
    conf.DisplayX=atol(root->first_node("DisplayX")->value());
    conf.DisplayY=atol(root->first_node("DisplayY")->value());
    conf.iframeX=atol(root->first_node("iframeX")->value());
    conf.iframeY=atol(root->first_node("iframeY")->value());
    conf.DefaultFontSize=atol(root->first_node("DefaultFontSize")->value());
    conf.layerACC=atol(root->first_node("LayerACC")->value());
    conf.layerTOP=atol(root->first_node("LayerTOP")->value());
    conf.layerBOT=atol(root->first_node("LayerBOT")->value());
    conf.layerROL=atol(root->first_node("LayerROL")->value());
    conf.DefaultFontName=string(root->first_node("DefaultFontName")->value());
    conf.VideoRatio=string(root->first_node("VideoRatio")->value());
    string XmlPath=string(root->first_node("XMLUri")->value());
    conf.ExportLocation=string(root->first_node("ExportLocation")->value());
    conf.AlphaTOP=atof(root->first_node("AlphaTOP")->value());
    conf.AlphaBOT=atof(root->first_node("AlphaBOT")->value());
    conf.AlphaROL=atof(root->first_node("AlphaROL")->value());
    conf.AlphaACC=atof(root->first_node("AlphaACC")->value());
    conf.MovingVelocity=atof(root->first_node("MovingVelocity")->value());
    conf.DynamicLengthRatio=atof(root->first_node("DynamicCompare")->value());
    conf.FontNameHei=BilibiliFont_Name_HEI;
    conf.FontNameYouYuan=BilibiliFont_Name_YouYuan;
    conf.FontNameSong=BilibiliFont_Name_Song;
    conf.FontNameKai=BilibiliFont_Name_Kai;
    conf.FontNameMSHei=BilibiliFont_Name_MS_HEI;
    conf.ContentType="Bilibili_XML";
    conf.XMLUrl=XmlPath;
    //Additional Support Ver. 1.2.0//////////////////////////////////////////////////////////////////////
    if(root->first_node("AllowVisitorComment")!=nullptr){
        conf.Allow_VisitorComment=ToBool(root->first_node("AllowVisitorComment")->value());
    }else{
        conf.Allow_VisitorComment=true;
    }
    for (xml_node<>* keyword_note=root->first_node("RemoveKeyword"); keyword_note!=NULL; keyword_note=keyword_note->next_sibling()) {
        if (string(keyword_note->name())!="RemoveKeyword") {continue;}
        conf.remove_key.push_back(keyword_note->value());
    }
    for (xml_node<>* color_note=root->first_node("RemoveColor"); color_note!=NULL; color_note=color_note->next_sibling()) {
        if (string(color_note->name())!="RemoveColor") {continue;}
        conf.remove_color.push_back(color_note->value());
    }
    for (xml_node<>* pool_note=root->first_node("RemovePool"); pool_note!=NULL; pool_note=pool_note->next_sibling()) {
        if (string(pool_note->name())!="RemovePool") {continue;}
        conf.remove_pool.push_back(pool_note->value());
    }
    for (xml_node<>* user_note=root->first_node("RemoveUser"); user_note!=NULL; user_note=user_note->next_sibling()) {
        if (string(user_note->name())!="RemoveUser") {continue;}
        conf.remove_user.push_back(user_note->value());
    }
    if (root->first_node("History")!=nullptr) {
        conf.comment_at_timestamp=atoi(root->first_node("History")->value());
    }else{
        conf.comment_at_timestamp=BILIBILI_NO_SPECIFIC_TIMESTAMP;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    doc.clear();
    const string pfx1("comment.bilibili.tv/");
    const string pfx2("http://comment.bilibili.tv/");
    string content;
    if (XmlPath.find(pfx1)==0||XmlPath.find(pfx2)==0){//Get though Internet
        content="";
        char errorBuffer[CURL_ERROR_SIZE];
        CURL *h=curl_easy_init();
        if (h==NULL) {
            throw runtime_error("Failed to Init CURL");
        }
        curl_easy_setopt(h,CURLOPT_READFUNCTION,NULL);
        curl_easy_setopt(h,CURLOPT_HEADERFUNCTION,NULL);
        curl_easy_setopt(h,CURLOPT_WRITEFUNCTION,_onCurlWrite);
        curl_easy_setopt(h,CURLOPT_WRITEDATA,&content);
        curl_easy_setopt(h,CURLOPT_ERRORBUFFER,errorBuffer);
        
        curl_easy_setopt(h,CURLOPT_URL,XmlPath.c_str());
        curl_easy_setopt(h,CURLOPT_FOLLOWLOCATION,1);
        curl_easy_setopt(h,CURLOPT_NOSIGNAL,1);
        curl_easy_setopt(h,CURLOPT_CONNECTTIMEOUT,3);
        curl_easy_setopt(h,CURLOPT_TIMEOUT,7);
        curl_easy_setopt(h,CURLOPT_ENCODING,"");
        CURLcode code=curl_easy_perform(h);
        curl_easy_cleanup(h);
        if (code!=CURLE_OK)
            throw runtime_error("Fail to get XML: "+string(errorBuffer));
        if (content=="")
            throw runtime_error("Corrupted XML: empty file");
    }else{//Get from local file
        content="";char ch;
        ifstream in(XmlPath.c_str());
        if (!in)
            throw runtime_error("Corrupted XML: Can't open file: "+XmlPath);
        while (!in.eof()){
            in.get(ch);
            content+=ch;
        }
        in.close();
    }
    //Additional Support Ver. 1.2.0//////////////////////////////////////////////////////////////////////
    if (conf.comment_at_timestamp!=BILIBILI_NO_SPECIFIC_TIMESTAMP) {
        char *temp_check_c_str=(char*)malloc(content.size()+1);
        strcpy(temp_check_c_str, content.c_str());
        xml_document<> check_doc;
        doc.parse<0>(temp_check_c_str);
        xml_node<>* check_root_node=doc.first_node();
        string temp_cid=check_root_node->first_node("chatid")->value();
        free(temp_check_c_str);
        const string list_constructor=string("http://comment.bilibili.com/rolldate,");
        string list_str=list_constructor+temp_cid;
        string check_list_content="";
        char errorBuffer[CURL_ERROR_SIZE];
        CURL *h=curl_easy_init();
        if (h==NULL) {
            throw runtime_error("Failed to Init CURL");
        }
        curl_easy_setopt(h,CURLOPT_READFUNCTION,NULL);
        curl_easy_setopt(h,CURLOPT_HEADERFUNCTION,NULL);
        curl_easy_setopt(h,CURLOPT_WRITEFUNCTION,_onCurlWrite);
        curl_easy_setopt(h,CURLOPT_WRITEDATA,&check_list_content);
        curl_easy_setopt(h,CURLOPT_ERRORBUFFER,errorBuffer);
        curl_easy_setopt(h,CURLOPT_URL,list_str.c_str());
        curl_easy_setopt(h,CURLOPT_FOLLOWLOCATION,1);
        curl_easy_setopt(h,CURLOPT_NOSIGNAL,1);
        curl_easy_setopt(h,CURLOPT_CONNECTTIMEOUT,3);
        curl_easy_setopt(h,CURLOPT_TIMEOUT,7);
        curl_easy_setopt(h,CURLOPT_ENCODING,"");
        CURLcode code=curl_easy_perform(h);
        curl_easy_cleanup(h);
        if (code!=CURLE_OK) {throw runtime_error("Fail to get XML [History List]: "+string(errorBuffer));}
        if (content=="") {throw runtime_error("Unable to get the history comment list.");}
        Document json_doc;
        json_doc.Parse<0>(check_list_content.c_str());
        if (json_doc.HasParseError()) {
            throw std::runtime_error("Fail to Parse the History List");
        }
        assert(json_doc.IsArray());
        bool time_matched=false;
        long matched_timestamp=BILIBILI_NO_SPECIFIC_TIMESTAMP;
        if (json_doc.IsArray()) {
            for (int i=0; i<json_doc.Size(); i++) {
                const Value& timestamp_node=json_doc[i]["timestamp"];
                //const Value& num_new_node=json_doc[i]["new"];
                const char* timestamp=timestamp_node.GetString();
                //const char* num_new=num_new_node.GetString();
                if (conf.comment_at_timestamp==atol(timestamp)){time_matched=true;}
                if (atol(timestamp)<conf.comment_at_timestamp) {
                    matched_timestamp=atol(timestamp);
                }
            }
        }
        if ((!time_matched)&&(matched_timestamp!=BILIBILI_NO_SPECIFIC_TIMESTAMP)) {
            cerr<<YellowOutput("No timestamp matched, will switch to the cloest timestamp.")<<endl;
            cerr<<YellowOutput(string("Current selected timestamp: ")+INT2STR(matched_timestamp))<<endl;
            conf.comment_at_timestamp=matched_timestamp;
        }
        if ((!time_matched)&&(matched_timestamp==BILIBILI_NO_SPECIFIC_TIMESTAMP)){
            throw std::runtime_error("Cannot find suitable timestamp.");
        }
        const string roll_back_url_const("http://comment.bilibili.com/dmroll,");
        string roll_back_url=roll_back_url_const+INT2STR(conf.comment_at_timestamp)+","+temp_cid;
        conf.XMLUrl=roll_back_url;
        string new_content="";
        char errorBuffer_2[CURL_ERROR_SIZE];
        CURL *h2=curl_easy_init();
        if (h2==NULL) {
            throw runtime_error("Failed to Init CURL");
        }
        curl_easy_setopt(h2,CURLOPT_READFUNCTION,NULL);
        curl_easy_setopt(h2,CURLOPT_HEADERFUNCTION,NULL);
        curl_easy_setopt(h2,CURLOPT_WRITEFUNCTION,_onCurlWrite);
        curl_easy_setopt(h2,CURLOPT_WRITEDATA,&new_content);
        curl_easy_setopt(h2,CURLOPT_ERRORBUFFER,errorBuffer_2);
        curl_easy_setopt(h2,CURLOPT_URL,conf.XMLUrl.c_str());
        curl_easy_setopt(h2,CURLOPT_FOLLOWLOCATION,1);
        curl_easy_setopt(h2,CURLOPT_NOSIGNAL,1);
        curl_easy_setopt(h2,CURLOPT_CONNECTTIMEOUT,3);
        curl_easy_setopt(h2,CURLOPT_TIMEOUT,7);
        curl_easy_setopt(h2,CURLOPT_ENCODING,"");
        CURLcode code2=curl_easy_perform(h2);
        curl_easy_cleanup(h2);
        if (code2!=CURLE_OK) {throw runtime_error("Fail to get XML [History XML]: "+string(errorBuffer));}
        if (new_content=="") {throw runtime_error("Unable to get the history comment XML.");}
        content=new_content;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    conf.XMLContent=content;
}

void thread_worker(const char* file_path_str){
    try{
        AssConfig conf;
        FillConfigure(conf,file_path_str);
        AssCompressor worker(conf);         //Auto-Start work
        if (flags_printxmlconfig) {
            _withMultiThread_Lock
            worker.PrintConfig();
            _withMultiThread_Unlock
        }
        if (worker.ErrorMsg!=""){
            _withMultiThread_Lock;
            std::cerr<<RedOutput("An Error occured! Please verify your input BEFORE reporting the bug.")<<std::endl;
            std::cerr<<RedOutput(string("[Target] ")+file_path_str)<<endl;
            std::cerr<<RedOutput(string("[ERROR]  ")+worker.ErrorMsg)<<std::endl;
            _withMultiThread_Unlock;
            return;
        }
        _withMultiThread_Lock;
        if (conf.ExportLocation==""){
            string ass=worker.ASS;
            cout<<ass<<endl<<";";
        }else{
            cout<<GreenOutput(string("Work for config \"")+file_path_str+string("\" finished successfully."))<<endl;
        }
        _withMultiThread_Unlock;
    }catch(runtime_error &e){
        _withMultiThread_Lock;
        std::cerr<<RedOutput("An Error occured! Please verify your input BEFORE reporting the bug.")<<std::endl;
        std::cerr<<RedOutput(string("[Target] ")+file_path_str)<<endl;
        std::cerr<<RedOutput(string("[ERROR]  ")+e.what())<<std::endl;
        _withMultiThread_Unlock;
        return;
    }
}

int main(int argc, const char * argv[])
{
    vector<string> argv_str;
    for (int i=0; i<argc; i++) {
        argv_str.push_back(argv[i]);
    }
    //Will analysis the argc
    for (size_t i=0; i<argv_str.size(); i++){
        if (argv_str[i]=="--version") {flags_version=true;}
        if (argv_str[i]=="--withConfig") {flags_printxmlconfig=true;}
        if (argv_str[i]=="--help") {flags_help=true;}
        if (argv_str[i].find(".xml")!=argv_str[i].npos) {task_list.push_back(argv_str[i].c_str());}
        if (argv_str[i]=="--buildConfig") {flags_printbuildflags=true;}
    }
    
    if ((argv_str.size()<=1)or(flags_help)) {
        print_help(argv_str[0]);
        return 0;
    }
    if (flags_version) {
        print_copyright();
        return 0;
    }
    if (flags_printbuildflags) {
        print_buildConfig();
        return 0;
    }
    #ifdef _withMultiThread
        for (string& iterate:task_list){
            worker_list.push_back(thread(thread_worker,iterate.c_str()));
        }
        for (thread& t:worker_list){
            if (t.joinable()) {t.join();}
        }
    #else
        for (int i=0; i<task_list.size(); i++){
            thread_worker(task_list[i].c_str());
        }
    #endif
    return 0;
}

inline string INT2STR(long val){
    ostringstream out;
    out<<val;
    return out.str();
}