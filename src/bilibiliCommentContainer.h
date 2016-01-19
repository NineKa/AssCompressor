#ifndef AssCompressor_bilibiliCommentContainer_H
#define AssCompressor_bilibiliCommentContainer_H

#include "rapidjson/document.h"
#include "configure.h"
#include "pipeError.h"

#include <limits>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <utility>
#include <exception>
#include <stdexcept>
#include <cmath>

#ifdef DEBUG
#include <sstream>
#endif

/*==============================================================================
 CommentStructure:
       1        2     3     4     5        6     7     8
 <d p="[double],[int],[int],[int],[time_t],[int],[str],[int]">[Context here]</d>
 
            +---+---------------------------------------+
            | 1 | Time      (Unit:Second)               |
            | 2 | Type      (Unit:Int)                  |
            | 3 | FontSize  (Unit:Int Default:25)       |
            | 4 | Color     (Unit:Int)                  |
            | 5 | Timestamp (Unit: time_t)              |
            | 6 | Pool      (0 = Normal; 1 = Subtitle)  |
            | 7 | UserID    (String)                    |
            | 8 | CommentID (int)                       |
            +---+---------------------------------------+                     */

enum bilibiliCommentType{
    TopRollComment,             //XMLCode = 1
    BottomRollComment,          //XMLCode = 2
    BottomFixComment,           //XMLCode = 4
    TopFixComment,              //XMLCode = 5
    ReverseTopRollComment,      //XMLCode = 6
    FXComment,                  //XMLCode = 7
    CodeComment,                //XMLCode = 8  (NOT SUPPORT FOR CURRENT VERSION)
    Unknown_Comment             //otherwise    (NOT SUPPORT FOR CURRENT VERSION)
};
enum bilibiliCommentPool{
    NormalPool,
    SubtitlePool,
    Unknown_Pool
};
 
/*FXComment:  @http://www.lonniebest.com/ASCII/Art/?ID=2
                 POSITIVE Y DIRECTION
                 ^      _________________________
                 |     / _____________________  /|
                 |    / / ___________________/ / |
                 |   / / /| |               / /  |
                 |  / / / | |     ^  POSITIVE Z DIRECTION
                 | / / /| | |    /        / / /| |
                 |/ / / | | |   /        / / / | |
                 | / /  | | |  /        / / /| | |
                /|/_/_________/________/ / / | | |
ORIGIN POINT ->/________________________/ /  | | |
               | ______________________ | |  | | |
               | | |    | |/|_________| | |__| | |
               | | |    | / __________| | |____| |
               | | |   / / ___________| | |_  / /
               | | |  / / /           | | |/ / /
               | | | / / /            | | | / /
               | | |/ / /             | | |/ /
               | | | / /              | | ' /
               | | |/_/_______________| |  /
               | |____________________| | /---------------> POSITIVE X DIRECTION
               |________________________|/
 
 
 Rotation:      Y rotation -> Z rotation
 Postive Z rotation:    Y axis -> X axis
 Postive Y rotation:    X axis -> Z axis
 
 Fade-in and Fade-out:
          0                                                             1
 Invisible<------------------------------------------------------------->Visible
 
 SX = Start X pos    SY = Start Y pos     FI = fade-in          FO = fade-out
 TX = To X pos       TY = To Y pos        MD = move duration    MP = move pause
 HL = highligh       FN = fontname       *LM = linear move    **TL = trace line
 C  = Content        D  = duration        ZR = z rotate         YR = y rotate
    +------+---+---+-------+---+---+---+---+---+---+---+---+---+---+---+---+
    | Type |0  |1  |2      |3  |4  |5  |6  |7  |8  |9  |10 |11 |12 |13 |14 |
    +------+---+---+-------+---+---+---+---+---+---+---+---+---+---+---+---+
    |ACC-05|SX |SY |FI-FO  |D  |C  |---|---|---|---|---|---|---|---|---|---|
    |ACC-07|SX |SY |FI-FO  |D  |C  |ZR |YR |---|---|---|---|---|---|---|---|
    |ACC-11|SX |SY |FI-FO  |D  |C  |ZR |YR |TX |TY |MD |MP |---|---|---|---|
    |ACC-13|SX |SY |FI-FO  |D  |C  |ZR |YR |TX |TY |MD |MP |FN |LM |---|---|
    |ACC-14|SX |SY |FI-FO  |D  |C  |ZR |YR |TX |TY |MD |MP |HL |FN |LM |---|
    |ACC-15|SX |SY |FI-FO  |D  |C  |ZR |YR |TX |TY |MD |MP |HL |FN |LM |TL |
    +------+---+---+-------+---+---+---+---+---+---+---+---+---+---+---+---+
  *linear move is not support due to ASS format restriction
 **Support of trace line movement will be highly resource consuming
==============================================================================*/
enum FXCommentType{
    ACC_05,                 //ACC-05 Format
    ACC_07,                 //ACC-07 Format
    ACC_11,                 //ACC-11 Format
    ACC_13,                 //ACC-13 Format
    ACC_14,                 //ACC-14 Format
    ACC_15,                 //ACC-15 Format
    Unknown_Format          //Not Support Version
};

class bilibiliCommentContainer{
    double              time = std::numeric_limits<double>::min();
    bilibiliCommentType type = bilibiliCommentType::Unknown_Comment;
    int                 fontsize = std::numeric_limits<int>::min();
    int                 color    = std::numeric_limits<int>::min();
    time_t              timestamp = std::numeric_limits<time_t>::max();
    bilibiliCommentPool pool = bilibiliCommentPool::Unknown_Pool;
    std::string         userID = std::string("");
    int                 commentID = std::numeric_limits<int>::min();
    std::string         content = std::string("");
    
    static void splitString(std::vector<std::string>& _array,
                            std::string& _data,
                            char _mark){
        std::string buffer = "";
        for (std::string::iterator iter = _data.begin();
             iter != _data.end();
             iter ++) {
            if (*iter != _mark) {
                buffer.append(1, *iter);
            }else{
                _array.push_back(buffer);
                buffer = "";
            }
        }
        if (!buffer.empty()) _array.push_back(buffer);
    }
    
public:
    constexpr static double timeEmpty = std::numeric_limits<double>::min();
    constexpr static bilibiliCommentType typeEmpty =
                                           bilibiliCommentType::Unknown_Comment;
    constexpr static int fontsizeEmpty = std::numeric_limits<int>::min();
    constexpr static int colorEmpty    = std::numeric_limits<int>::min();
    constexpr static time_t timestampEmpty = std::numeric_limits<time_t>::max();
    constexpr static bilibiliCommentPool poolEmpty =
                                              bilibiliCommentPool::Unknown_Pool;
    constexpr static int commentIDEmpty = std::numeric_limits<int>::min();
    
    constexpr static size_t invalidPos  = std::numeric_limits<size_t>::max();
    constexpr static double invalidFade = std::numeric_limits<size_t>::min();
    constexpr static double invalidDuration =std::numeric_limits<double>::min();
    constexpr static double invalidRotate   =std::numeric_limits<double>::min();
    
    struct fxCommentInfo{
        FXCommentType type = FXCommentType::Unknown_Format;             //Type
        size_t start_X = bilibiliCommentContainer::invalidPos;          //SX
        size_t start_Y = bilibiliCommentContainer::invalidPos;          //SY
        double fadeIn  = bilibiliCommentContainer::invalidFade;         //FI
        double fadeOut = bilibiliCommentContainer::invalidFade;         //FO
        size_t to_X    = bilibiliCommentContainer::invalidPos;          //TX
        size_t to_Y    = bilibiliCommentContainer::invalidPos;          //TY
        double moveDur = bilibiliCommentContainer::invalidDuration;     //MD
        double movePau = bilibiliCommentContainer::invalidDuration;     //MP
        bool   highlight = false;                                       //HL
        std::string fontname = std::string("");                         //FN
        std::string content  = std::string("");                         //C
        double duration = bilibiliCommentContainer::invalidDuration;    //D
        double zRotate  = bilibiliCommentContainer::invalidRotate;      //ZR
        double yRotate  = bilibiliCommentContainer::invalidRotate;      //YR
        #ifdef bilibiliCommentContainer_SupportTraceLine
        std::vector<std::pair<size_t, size_t>> traceline;               //TL
        #endif
    };
    
    fxCommentInfo      fxInfo;
    
    double getTime(){return this->time;}
    bilibiliCommentType getType(){return this->type;}
    int    getFontSize(){return this->fontsize;}
    int    getColor(){return this->color;}
    time_t getTimestamp(){return this->timestamp;}
    bilibiliCommentPool getPool(){return this->pool;}
    std::string getUserID(){return this->userID;}
    int    getCommentID(){return this->commentID;}
    std::string getContent(){
        if (type == bilibiliCommentType::FXComment) {
            return fxInfo.content;
        }else{
            return content;
        }
    }
    
    static bilibiliCommentType parseTypeInt(int _type){
        switch (_type) {
            case 1:  return bilibiliCommentType::TopRollComment;
            case 2:  return bilibiliCommentType::BottomRollComment;
            case 4:  return bilibiliCommentType::BottomFixComment;
            case 5:  return bilibiliCommentType::TopFixComment;
            case 6:  return bilibiliCommentType::ReverseTopRollComment;
            case 7:  return bilibiliCommentType::FXComment;
            case 8:  return bilibiliCommentType::CodeComment;
            default: return bilibiliCommentType::Unknown_Comment;
        }
    }
    
    static bilibiliCommentPool parsePoolInt(int _pool){
        switch (_pool) {
            case 0:  return bilibiliCommentPool::NormalPool;
            case 1:  return bilibiliCommentPool::SubtitlePool;
            default: return bilibiliCommentPool::Unknown_Pool;
        }
    }
    
    static FXCommentType parseFXCommentInt(int _size){
        switch (_size) {
            case 5:  return FXCommentType::ACC_05;
            case 7:  return FXCommentType::ACC_07;
            case 11: return FXCommentType::ACC_11;
            case 13: return FXCommentType::ACC_13;
            case 14: return FXCommentType::ACC_14;
            case 15: return FXCommentType::ACC_15;
            default: return FXCommentType::Unknown_Format;
        }
    }
    
    static std::pair<double, double> parseFI_FO(const char* _str){
        std::string parseStr(_str);
        std::vector<std::string> returnVec;
        splitString(returnVec, parseStr, '-');
        if (returnVec.size() != 2) {
            throw std::runtime_error(BILIBILICOMMENTCONTAINER_PARSE_ERROR);
        }
        std::pair<double, double> returnPair(atof(returnVec[0].c_str()),
                                             atof(returnVec[1].c_str()));
        return returnPair;
    }
    
    static std::pair<int,int> parseInt_Double_Str_Pos(rapidjson::Value& _nodeX,
                                                      rapidjson::Value& _nodeY,
                                                      const int _displayX,
                                                      const int _displayY){
        assert(_nodeX.IsNumber() || _nodeX.IsString());
        assert(_nodeY.IsNumber() || _nodeY.IsString());
        int returnX = 0;
        int returnY = 0;
        if (_nodeX.IsDouble()) {
            returnX = static_cast<int>(floor(_nodeX.GetDouble() * _displayX));
        }else if (_nodeX.IsString()){
            returnX = atoi(_nodeX.GetString());
        }else if (_nodeX.GetInt() == 1){
            returnX = _displayX;
        }else returnX = _nodeX.GetInt();
        if (_nodeY.IsDouble()) {
            returnY = static_cast<int>(floor(_nodeY.GetDouble() * _displayY));
        }else if (_nodeY.IsString()){
            returnY = atoi(_nodeY.GetString());
        }else if (_nodeY.GetInt() == 1){
            returnY = _displayY;
        }else returnY = _nodeY.GetInt();
        std::pair<int,int> returnPair(returnX, returnY);
        return returnPair;
    }
    
    static void fxAnalysis_ACC05(fxCommentInfo& _info,
                                 rapidjson::Document& _d,
                                 const int _displayX,
                                 const int _displayY){
        assert(_d[0].IsNumber() || _d[0].IsString());   //SX
        assert(_d[1].IsNumber() || _d[0].IsString());   //SY
        assert(_d[2].IsString());                       //FI-FO
        assert(_d[3].IsNumber() || _d[0].IsString());   //D
        assert(_d[4].IsString());                       //C
        std::pair<int,int> pos = parseInt_Double_Str_Pos(_d[0], _d[1],
                                                         _displayX, _displayY);
        _info.type = FXCommentType::ACC_05;
        assert(pos.first>=0 && pos.second>=0);
        _info.start_X = static_cast<size_t>(pos.first);
        _info.start_Y = static_cast<size_t>(pos.second);
        std::pair<double,double> fade = parseFI_FO(_d[2].GetString());
        assert(fade.first >=0 && fade.first <=1);
        assert(fade.second >=0 && fade.second <=1);
        _info.fadeIn = fade.first;
        _info.fadeOut = fade.second;
        _info.duration = (_d[3].IsDouble())?
                   _d[3].GetDouble():(_d[3].IsString()? atof(_d[3].GetString()):
                   static_cast<double>(_d[3].GetInt()));
        _info.content = std::string(_d[4].GetString());
    }
    
    static void fxAnalysis_ACC07(fxCommentInfo& _info,
                                 rapidjson::Document& _d,
                                 const int _displayX,
                                 const int _displayY){
        fxAnalysis_ACC05(_info, _d, _displayX, _displayY);
        _info.type = FXCommentType::ACC_07;
        assert(_d[5].IsInt() || _d[5].IsString());       //ZR
        assert(_d[6].IsInt() || _d[6].IsString());       //YR
        _info.zRotate = _d[5].IsString()?atoi(_d[5].GetString()):_d[5].GetInt();
        _info.yRotate = _d[6].IsString()?atoi(_d[6].GetString()):_d[6].GetInt();
    }
    
    static void fxAnalysis_ACC11(fxCommentInfo& _info,
                                 rapidjson::Document& _d,
                                 const int _displayX,
                                 const int _displayY){
        fxAnalysis_ACC07(_info, _d, _displayX, _displayY);
        _info.type = FXCommentType::ACC_11;
        assert(_d[7].IsNumber() || _d[7].IsString());   //TX
        assert(_d[8].IsNumber() || _d[8].IsString());   //TY
        assert(_d[9].IsInt() || _d[9].IsString());      //MD
        assert(_d[10].IsInt() || _d[10].IsString());    //MP
        std::pair<int, int> tPos = parseInt_Double_Str_Pos(_d[7], _d[8],
                                                          _displayX, _displayY);
        assert(tPos.first >=0 && tPos.second >=0);
        _info.to_X = static_cast<size_t>(tPos.first);
        _info.to_Y = static_cast<size_t>(tPos.second);
        _info.moveDur = (_d[9].IsInt()?static_cast<double>(_d[9].GetInt()):
                                       atof(_d[9].GetString())) / 1000.0;
        _info.movePau = (_d[10].IsInt()?static_cast<double>(_d[10].GetInt()):
                                       atof(_d[10].GetString()))/ 1000.0;
    }
    
    static void fxAnalysis_ACC13(fxCommentInfo& _info,
                                 rapidjson::Document& _d,
                                 const int _displayX,
                                 const int _displayY){
        fxAnalysis_ACC11(_info, _d, _displayX, _displayY);
        _info.type = FXCommentType::ACC_13;
        assert(_d[11].IsString());                      //FN
        assert(_d[12].IsInt() || _d[12].IsString());    //LM    (NOT SUPPORT)
        _info.fontname = std::string(_d[11].GetString());
    }
    
    static void fxAnalysis_ACC14(fxCommentInfo& _info,
                                 rapidjson::Document& _d,
                                 const int _displayX,
                                 const int _displayY){
        fxAnalysis_ACC11(_info, _d, _displayX, _displayY);
        _info.type = FXCommentType::ACC_14;
        assert(_d[11].IsBool() || _d[11].IsString());   //HL
        assert(_d[12].IsString());                      //FN
        assert(_d[13].IsInt()  || _d[13].IsString());   //LM    (NOT SUPPORT)
        _info.highlight = _d[11].IsBool()?
                          ((_d[11].GetBool())? true: false) :
                          (strcmp(_d[13].GetString(), "true")==0? true : false);
        _info.fontname = std::string(_d[12].GetString());
    }
    
    #ifdef bilibiliCommentContainer_SupportTraceLine
    static void fxAnalysis_ACC15(fxCommentInfo& _info,
                                 rapidjson::Document& _d,
                                 const int _displayX,
                                 const int _displayY){
        fxAnalysis_ACC14(_info, _d, _displayX, _displayY);
        _info.type = FXCommentType::ACC_15;
        assert(_d[14].IsString());                      //LM
        std::string linearTrack = std::string(_d[14].GetString());
        linearTrack = linearTrack.substr(1,linearTrack.length()-1);
        std::vector<std::string> TrackArray;
        splitString(TrackArray, linearTrack, 'L');
        for (std::vector<std::string>::iterator iter = TrackArray.begin();
             iter != TrackArray.end(); iter++) {
            std::string& posStr = *iter;
            std::vector<std::string> posSet;
            splitString(posSet, posStr, ',');
            assert(posSet.size() == 2);
            std::pair<size_t, size_t> pos((size_t)(atol(posSet[0].c_str())),
                                          (size_t)(atol(posSet[1].c_str())));
            _info.traceline.push_back(pos);
        }
    }
    #endif
    
    static void fxAnalysis(fxCommentInfo& _info,
                           std::string& _content,
                           const int _displayX,
                           const int _displayY){
        char* buffer = (char*)malloc(sizeof(char)*(_content.length()+1));
        strcpy(buffer, _content.c_str());
        rapidjson::Document jsonParser;
        jsonParser.Parse<rapidjson::kParseDefaultFlags>(buffer);
        if (jsonParser.HasParseError()) {
            free(buffer);
            throw std::runtime_error(BILIBILICOMMENTCONTAINER_PARSE_ERROR);
        }
        if (!jsonParser.IsArray()) {
            free(buffer);
            throw std::runtime_error(BILIBILICOMMENTCONTAINER_PARSE_ERROR);
        }
        FXCommentType type = parseFXCommentInt(jsonParser.Size());
        _info.type = type;
        switch (_info.type) {
            case ACC_05: fxAnalysis_ACC05(_info,jsonParser,_displayX,_displayY);
                break;
            case ACC_07: fxAnalysis_ACC07(_info,jsonParser,_displayX,_displayY);
                break;
            case ACC_11: fxAnalysis_ACC11(_info,jsonParser,_displayX,_displayY);
                break;
            case ACC_13: fxAnalysis_ACC13(_info,jsonParser,_displayX,_displayY);
                break;
            case ACC_14: fxAnalysis_ACC14(_info,jsonParser,_displayX,_displayY);
                break;
            #ifdef bilibiliCommentContainer_SupportTraceLine
            case ACC_15: fxAnalysis_ACC15(_info,jsonParser,_displayX,_displayY);
                break;
            #else
            case ACC_15:
                throw std::runtime_error(BILIBILICOMMENTCONTAINER_NOTSUPPORT);
            #endif
            default:
                throw std::runtime_error(BILIBILICOMMENTCONTAINER_PARSE_ERROR);
        }
        free(buffer);
    }
    
    bilibiliCommentContainer(std::string& _attribute,
                             std::string& _data,
                             const int _displayX,
                             const int _displayY){
        std::vector<std::string> attributeArray;
        bilibiliCommentContainer::splitString(attributeArray, _attribute, ',');
        if (attributeArray.size()!=bilibiliCommentContainer_WellFormAttribute){
            throw std::runtime_error(BILIBILICOMMENTCONTAINER_PARSE_ERROR);
        }
        this->time = atof(attributeArray[0].c_str());
        this->type = parseTypeInt(atoi(attributeArray[1].c_str()));
        this->fontsize = atoi(attributeArray[2].c_str());
        this->color = atoi(attributeArray[3].c_str());
        this->timestamp = static_cast<time_t>(atol(attributeArray[4].c_str()));
        this->pool = parsePoolInt(atoi(attributeArray[5].c_str()));
        this->userID = attributeArray[6];
        this->commentID = atoi(attributeArray[7].c_str());
        this->content = _data;
        
        if (this->type == bilibiliCommentType::FXComment){
            fxAnalysis(this->fxInfo, this->content,
                       _displayX, _displayY);
        }
        if (this->type == bilibiliCommentType::Unknown_Comment){
            throw std::runtime_error(BILIBILICOMMENTCONTAINER_PARSE_ERROR);
        }
        if (this->type == bilibiliCommentType::CodeComment){
            throw std::runtime_error(BILIBILICOMMENTCONTAINER_NOTSUPPORT);
        }
    }

    static std::string commentType2Str(bilibiliCommentType& _type){
        switch (_type) {
            case bilibiliCommentType::TopRollComment:
                return std::string("TopRoll");
            case bilibiliCommentType::BottomRollComment:
                return std::string("BottomRoll");
            case bilibiliCommentType::BottomFixComment:
                return std::string("BottomFix");
            case bilibiliCommentType::TopFixComment:
                return std::string("TopFix");
            case bilibiliCommentType::ReverseTopRollComment:
                return std::string("ReverseTop");
            case bilibiliCommentType::FXComment:
                return std::string("FX");
            case bilibiliCommentType::CodeComment:
                return std::string("Code");
            case bilibiliCommentType::Unknown_Comment:
                return std::string("Unknown");
        }
    }
    
    static std::string commentPool2Str(bilibiliCommentPool& _pool){
        switch (_pool) {
            case bilibiliCommentPool::NormalPool  :
                return std::string("Normal");
            case bilibiliCommentPool::SubtitlePool:
                return std::string("Subtitle");
            case bilibiliCommentPool::Unknown_Pool:
                return std::string("Unknown");
        }
    }
    
    static std::string fxCommentType2Str(FXCommentType& _type){
        switch (_type) {
            case FXCommentType::ACC_05: return std::string("ACC_05");
            case FXCommentType::ACC_07: return std::string("ACC_07");
            case FXCommentType::ACC_11: return std::string("ACC_11");
            case FXCommentType::ACC_13: return std::string("ACC_13");
            case FXCommentType::ACC_14: return std::string("ACC_14");
            case FXCommentType::ACC_15: return std::string("ACC_15");
            case FXCommentType::Unknown_Format:
                return std::string("Unknown");
        }
    }
    
    static std::string fxInfo2Str(fxCommentInfo& _info){
        std::string returnStr;
        std::stringstream oStream;
        oStream<<"ACCType:"<<fxCommentType2Str(_info.type);
        oStream<<",StartX:"<<_info.start_X;
        oStream<<",StartY:"<<_info.start_Y;
        oStream<<",FadeIN:"<<_info.fadeIn;
        oStream<<",FadeOUT:"<<_info.fadeOut;
        oStream<<",Duration"<<_info.duration;
        if (_info.type == FXCommentType::ACC_05){
            oStream>>returnStr;
            return returnStr;
        }
        oStream<<",ZRotate:"<<_info.zRotate;
        oStream<<",YRotate:"<<_info.yRotate;
        if (_info.type == FXCommentType::ACC_07){
            oStream>>returnStr;
            return returnStr;
        }
        oStream<<",ToX:"<<_info.to_X;
        oStream<<",ToY:"<<_info.to_Y;
        oStream<<",MovingDuration:"<<_info.moveDur;
        oStream<<",MovingPause:"<<_info.movePau;
        if (_info.type == FXCommentType::ACC_11){
            oStream>>returnStr;
            return returnStr;
        }
        if (_info.type == FXCommentType::ACC_13){
            oStream<<",Fontname:"<<_info.fontname;
            oStream>>returnStr;
            return returnStr;
        }
        oStream<<",Highlight:"<<(_info.highlight ? "true" : "false");
        oStream<<",Fontname:"<<_info.fontname;
        if (_info.type == FXCommentType::ACC_14){
            oStream>>returnStr;
            return returnStr;
        }
        oStream<<",TraceLine:";
        for (std::vector<std::pair<size_t,size_t>>::iterator iter =
                                                        _info.traceline.begin();
             iter != _info.traceline.end(); iter++) {
            oStream<<"("<<iter->first<<","<<iter->second<<")->";
        }
        oStream>>returnStr;
        return returnStr.substr(0, returnStr.length()-2);
    }
    
    std::string toString(){
        std::stringstream oStream;
        oStream<<"[Time:"<<this->time<<",Type:"<<commentType2Str(this->type);
        oStream<<",Fontsize:"<<this->fontsize<<",Color:"<<this->color;
        oStream<<",Timestamp:"<<this->timestamp;
        oStream<<",Pool:"<<commentPool2Str(this->pool);
        oStream<<",UserID:"<<this->userID;
        oStream<<",Comment:"<<this->commentID;
        oStream<<",Content:"<<this->getContent();
        
        if (this->type == bilibiliCommentType::FXComment) {
            oStream<<",FXInfo:{"<<fxInfo2Str(this->fxInfo)<<"}";
        }
        
        oStream<<"]";
        char* buffer = (char*)malloc(sizeof(char)*1024);
        oStream.getline(buffer, 1024);
        std::string returnStr (buffer);
        free(buffer);
        return returnStr;
    }
};



#endif