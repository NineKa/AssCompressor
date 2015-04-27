#ifndef __Class__BiliBiliCommentContainer__
#define __Class__BiliBiliCommentContainer__

#include <string>
#include <cstring>
#include <regex>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <regex>
#include "rapidjson/rapidjson.h"

#define Error_BiliBiliCommentContainer_Analysis "Unable to Decode Commnet"
#define Error_BiliBiliCommentContainer_DecodeACC "Unable to Decode ACC Comment"

struct TracePos {int posX = 0;  int posY = 0;};

struct BiliBiliCommentContainer_PositionedDetail{
    int initPosX = 0; int initPosY = 0;
    int movePosX = 0; int movePosY = 0;
    double fadeinColor = 0.00;
    double fadeoutColor = 0.00;
    double duration = 0.00;
    int rotateZ = 0;  int rotateY = 0;
    long movingDurationMS = 0;
    long movingPauseMS = 0;
    bool highlight = false;
    bool linearAcc = false;
    std::string fontName = "";
    std::vector<TracePos> traceLine;
};

class BiliBiliCommentContainer{
public:
    enum BiliBiliCommentType{
        type_rollComment,
        type_topComment,
        type_bottomComment,
        type_bottomRollComment,
        type_reverseComment,
        type_positionedComment,
        type_codeComment,
        type_unknown
    };
    enum BilibiliCommentPool{
        pool_normalPool,
        pool_subtitlePool,
        pool_unknown
    };
    BiliBiliCommentContainer(){};
    BiliBiliCommentContainer(std::string& _attribute, std::string& _data){this->analysis(_attribute, _data);}
    void analysis(std::string&, std::string&);
    static std::string decimalToHexColor(unsigned long&);
    
    const BiliBiliCommentType getType(){return this->type;}
    const BilibiliCommentPool getPool(){return this->pool;}
    const double getTime(){return this->time;}
    const time_t getTimestamp(){return this->timestamp;}
    const unsigned long getFontSize(){return this->fontSize;}
    const unsigned long getFontColor(){return this->fontColor;}
    std::string getUserID(){return this->userID;}
    std::string getCommentID(){return this->commentID;}
    std::string getContent(){return this->content;}
    BiliBiliCommentContainer_PositionedDetail& getPositionedDetial(){return this->additionalInfo;}
    
    const bool isVisitorComment(){return (this->userID[0] == 'D')?true:false;}
    
    void setType(const BiliBiliCommentType _type){this->type = _type; return;}
    void setPool(const BilibiliCommentPool _pool){this->pool = _pool; return;}
    void setTimestamp(const time_t _timestamp){this->timestamp = _timestamp; return;}
    void setTime(const double _time){this->time = _time; return;}
    void setFontSize(const unsigned long _fontsize){this->fontSize = _fontsize; return;}
    void setFontColor(const unsigned long _fontcolor){this->fontColor = _fontcolor; return;}
    void setUserID(std::string _userid){this->userID = _userid; return;}
    void setCommentID(std::string _commentid){this->commentID = _commentid; return;}
    void setContent(std::string _content){this->content = _content; return;}
    void setPostitionedDetial(BiliBiliCommentContainer_PositionedDetail& _detail){this->additionalInfo = _detail; return;}
    
    static BiliBiliCommentType getTypeByStr(const char*);
    static BilibiliCommentPool getPoolByStr(const char*);
    static std::string getStrByType(BiliBiliCommentType);
    static std::string getStrByPool(BilibiliCommentPool);
protected:
    double time = 0;
    BiliBiliCommentType type = BiliBiliCommentType::type_rollComment;
    unsigned long fontSize = 25;
    unsigned long fontColor = 0xffffff;
    time_t timestamp = 0;
    BilibiliCommentPool pool = BilibiliCommentPool::pool_normalPool;
    std::string userID = "";
    std::string commentID = "";
    std::string content = "";
    BiliBiliCommentContainer_PositionedDetail additionalInfo;
private:
    void furtherAnalysis();
    inline void furtherAnalysis_Sub_ACC05(rapidjson::Document&);
    inline void furtherAnalysis_Sub_ACC07(rapidjson::Document&);
    inline void furtherAnalysis_Sub_ACC11(rapidjson::Document&);
    inline void furtherAnalysis_Sub_ACC13(rapidjson::Document&);
    inline void furtherAnalysis_Sub_ACC14(rapidjson::Document&);
    inline void furtherAnalysis_Sub_ACC15(rapidjson::Document&);
    inline static void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c){ //@egmkang
        std::string::size_type pos1, pos2;
        pos2 = s.find(c);
        pos1 = 0;
        while(std::string::npos != pos2)
        {
            v.push_back(s.substr(pos1, pos2-pos1));
            pos1 = pos2 + c.size();
            pos2 = s.find(c, pos1);
        }
        if(pos1 != s.length())
            v.push_back(s.substr(pos1));
    }
};

std::string BiliBiliCommentContainer::decimalToHexColor(unsigned long& _color){
    #define _decimalToHexColor_R1 (tempBuffer[0])
    #define _decimalToHexColor_R2 (tempBuffer[1])
    #define _decimalToHexColor_G1 (tempBuffer[2])
    #define _decimalToHexColor_G2 (tempBuffer[3])
    #define _decimalToHexColor_B1 (tempBuffer[4])
    #define _decimalToHexColor_B2 (tempBuffer[5])
    char* tempBuffer = new char[7];
    sprintf(tempBuffer, "%06lx", _color);
    tempBuffer[0] = _decimalToHexColor_B1;  //Output in ASS Standard BGR
    tempBuffer[1] = _decimalToHexColor_B2;
    tempBuffer[2] = _decimalToHexColor_G1;
    tempBuffer[3] = _decimalToHexColor_G2;
    tempBuffer[4] = _decimalToHexColor_R1;
    tempBuffer[5] = _decimalToHexColor_R2;
    std::string returnStr(tempBuffer);
    delete tempBuffer;
    return returnStr;
}

void BiliBiliCommentContainer::analysis(std::string& _attribute, std::string& _data){
    std::vector<std::string> captureGroup;
    BiliBiliCommentContainer::SplitString(_attribute, captureGroup, ",");
    captureGroup.push_back(_data);
    if (captureGroup.size() != 9){
        throw std::runtime_error(Error_BiliBiliCommentContainer_Analysis);
    }
    #define _analysis_capture_time          (captureGroup[0])
    #define _analysis_capture_type          (captureGroup[1])
    #define _analysis_capture_fontSize      (captureGroup[2])
    #define _analysis_capture_fontColor     (captureGroup[3])
    #define _analysis_capture_timestamp     (captureGroup[4])
    #define _analysis_capture_pool          (captureGroup[5])
    #define _analysis_capture_userID        (captureGroup[6])
    #define _analysis_capture_commentID     (captureGroup[7])
    #define _analysis_capture_content       (captureGroup[8])
    this->time = atof(std::string(_analysis_capture_time).c_str());
    this->type = BiliBiliCommentContainer::getTypeByStr(std::string(_analysis_capture_type).c_str());
    this->fontSize = static_cast<unsigned long>(atol(std::string(_analysis_capture_fontSize).c_str()));
    this->fontColor = static_cast<unsigned long>(atol(std::string(_analysis_capture_fontColor).c_str()));
    this->timestamp = static_cast<time_t>(atol(std::string(_analysis_capture_timestamp).c_str()));
    this->pool = BiliBiliCommentContainer::getPoolByStr(std::string(_analysis_capture_pool).c_str());
    this->userID = std::string(_analysis_capture_userID);
    this->commentID = std::string(_analysis_capture_commentID);
    this->content = std::string(_analysis_capture_content);
    if (this->type == BiliBiliCommentType::type_positionedComment){
        this->furtherAnalysis();
    }
}

void BiliBiliCommentContainer::furtherAnalysis(){
    std::string& source = this->content;
    rapidjson::Document ACCJsonArr;
    ACCJsonArr.Parse<0>(source.c_str());
    if (ACCJsonArr.HasParseError()) {
        throw std::runtime_error(Error_BiliBiliCommentContainer_DecodeACC);
    }
    switch (ACCJsonArr.Size()) {
        case 5: this->furtherAnalysis_Sub_ACC05(ACCJsonArr); return;
        case 7: this->furtherAnalysis_Sub_ACC07(ACCJsonArr); return;
        case 11:this->furtherAnalysis_Sub_ACC11(ACCJsonArr); return;
        case 13:this->furtherAnalysis_Sub_ACC13(ACCJsonArr); return;
        case 14:this->furtherAnalysis_Sub_ACC14(ACCJsonArr); return;
        case 15:this->furtherAnalysis_Sub_ACC15(ACCJsonArr); return;
        default:
            throw std::runtime_error(Error_BiliBiliCommentContainer_DecodeACC);
            return;
    }
}

void BiliBiliCommentContainer::furtherAnalysis_Sub_ACC05(rapidjson::Document& _doc){
    rapidjson::Value& nodePosX = _doc[0];
    rapidjson::Value& nodePosY = _doc[1];
    rapidjson::Value& nodeFadeStr = _doc[2];
    rapidjson::Value& nodeDuration = _doc[3];
    rapidjson::Value& nodeContent = _doc[4];
    this->additionalInfo.initPosX = (nodePosX.IsInt())?nodePosX.GetInt():atoi(nodePosX.GetString());
    this->additionalInfo.initPosY = (nodePosY.IsInt())?nodePosY.GetInt():atoi(nodePosY.GetString());
    this->additionalInfo.movePosX = (nodePosX.IsInt())?nodePosX.GetInt():atoi(nodePosX.GetString());
    this->additionalInfo.movePosY = (nodePosY.IsInt())?nodePosY.GetInt():atoi(nodePosY.GetString());
    std::string tempFadeStr = nodeFadeStr.GetString();
    std::vector<std::string> tempFadeArr;
    this->SplitString(tempFadeStr, tempFadeArr, "-");
    this->additionalInfo.fadeinColor = atof(tempFadeArr[0].c_str());
    this->additionalInfo.fadeoutColor = atof(tempFadeArr[1].c_str());
    this->additionalInfo.duration = (nodeDuration.IsDouble())?nodeDuration.GetDouble():((nodeDuration.IsInt())?nodeDuration.GetInt():atof(nodeDuration.GetString()));
    this->additionalInfo.movingDurationMS = static_cast<long>(this->additionalInfo.duration*1000);
    this->additionalInfo.movingPauseMS = 0;
    this->additionalInfo.traceLine.clear();
    this->content = nodeContent.GetString();
    return;
}

void BiliBiliCommentContainer::furtherAnalysis_Sub_ACC07(rapidjson::Document& _doc){
    rapidjson::Value& nodeZRotate = _doc[5];
    rapidjson::Value& nodeYRotate = _doc[6];
    this->furtherAnalysis_Sub_ACC05(_doc);
    this->additionalInfo.rotateZ = (nodeZRotate.IsInt())?nodeZRotate.GetInt():atoi(nodeZRotate.GetString());
    this->additionalInfo.rotateY = (nodeYRotate.IsInt())?nodeYRotate.GetInt():atoi(nodeYRotate.GetString());
    return;
}

void BiliBiliCommentContainer::furtherAnalysis_Sub_ACC11(rapidjson::Document& _doc){
    rapidjson::Value& nodeMovePosX = _doc[7];
    rapidjson::Value& nodeMovePosY = _doc[8];
    rapidjson::Value& nodeMoveDurationMS = _doc[9];
    rapidjson::Value& nodeMovePauseMS = _doc[10];
    this->furtherAnalysis_Sub_ACC07(_doc);
    this->additionalInfo.movePosX = nodeMovePosX.IsInt()?nodeMovePosX.GetInt():atoi(nodeMovePosX.GetString());
    this->additionalInfo.movePosY = nodeMovePosY.IsInt()?nodeMovePosY.GetInt():atoi(nodeMovePosY.GetString());
    this->additionalInfo.movingDurationMS = static_cast<long>(nodeMoveDurationMS.IsInt64()?nodeMoveDurationMS.GetInt64():atol(nodeMoveDurationMS.GetString()));
    this->additionalInfo.movingPauseMS = static_cast<long>(nodeMovePauseMS.IsInt64()?nodeMovePauseMS.GetInt64():atol(nodeMovePauseMS.GetString()));
    return;
}

void BiliBiliCommentContainer::furtherAnalysis_Sub_ACC13(rapidjson::Document& _doc){
    rapidjson::Value& nodeFontName = _doc[11];
    rapidjson::Value& nodeLinearAcc = _doc[12];
    this->furtherAnalysis_Sub_ACC11(_doc);
    this->additionalInfo.fontName = nodeFontName.GetString();
    int tempLinearAcc = nodeLinearAcc.IsInt()?nodeLinearAcc.GetInt():atoi(nodeLinearAcc.GetString());
    this->additionalInfo.linearAcc = (tempLinearAcc==0)?true:false;
    return;
}

void BiliBiliCommentContainer::furtherAnalysis_Sub_ACC14(rapidjson::Document& _doc){
    rapidjson::Value& nodeHighlight = _doc[11];
    rapidjson::Value& nodeFontName = _doc[12];
    rapidjson::Value& nodeLinearAcc = _doc[13];
    this->furtherAnalysis_Sub_ACC11(_doc);
    this->additionalInfo.highlight = nodeHighlight.IsBool()?nodeHighlight.GetBool():(strcmp(nodeHighlight.GetString(), "true")?true:false);
    this->additionalInfo.fontName = nodeFontName.GetString();
    int tempLinearAcc = nodeLinearAcc.IsInt()?nodeLinearAcc.GetInt():atoi(nodeLinearAcc.GetString());
    this->additionalInfo.linearAcc = (tempLinearAcc==0)?true:false;
    return;
}

void BiliBiliCommentContainer::furtherAnalysis_Sub_ACC15(rapidjson::Document& _doc){
    rapidjson::Value& nodeTraceLine = _doc[14];
    this->furtherAnalysis_Sub_ACC14(_doc);
    std::string tempTraceLine = nodeTraceLine.GetString();
    tempTraceLine = tempTraceLine.substr(1, tempTraceLine.length()-1);
    std::vector<std::string> coordinateCoupleArr;
    this->SplitString(tempTraceLine, coordinateCoupleArr, "L");
    for (std::string& i : coordinateCoupleArr){
        size_t posComma = i.find(",");
        TracePos appendNode;
        appendNode.posX = atoi(tempTraceLine.substr(0,posComma-1).c_str());
        appendNode.posY = atoi(tempTraceLine.substr(posComma+1, tempTraceLine.length()-1).c_str());
        this->additionalInfo.traceLine.push_back(appendNode);
    }
    return;
}

BiliBiliCommentContainer::BiliBiliCommentType BiliBiliCommentContainer::getTypeByStr(const char* _str){
    int tempInt = atoi(_str);
    switch (tempInt) {
        case 1: return BiliBiliCommentType::type_rollComment;
        case 2: return BiliBiliCommentType::type_bottomRollComment;
        case 4: return BiliBiliCommentType::type_bottomComment;
        case 5: return BiliBiliCommentType::type_topComment;
        case 6: return BiliBiliCommentType::type_reverseComment;
        case 7: return BiliBiliCommentType::type_positionedComment;
        case 8: return BiliBiliCommentType::type_codeComment;
        default:return BiliBiliCommentType::type_unknown;
    }
}

BiliBiliCommentContainer::BilibiliCommentPool BiliBiliCommentContainer::getPoolByStr(const char* _str){
    int tempInt = atoi(_str);
    switch (tempInt) {
        case 0: return BilibiliCommentPool::pool_normalPool;
        case 1: return BilibiliCommentPool::pool_subtitlePool;
        default:return BilibiliCommentPool::pool_unknown;
    }
}

std::string BiliBiliCommentContainer::getStrByType(BiliBiliCommentContainer::BiliBiliCommentType _type){
    switch (_type) {
        case type_bottomComment:        return std::string("bottomComment");
        case type_codeComment:          return std::string("codeComment");
        case type_positionedComment:    return std::string("positionedComment");
        case type_bottomRollComment:    return std::string("bottomRollComment");
        case type_reverseComment:       return std::string("reverseComment");
        case type_rollComment:          return std::string("rollComment");
        case type_topComment:           return std::string("topComment");
        case type_unknown:              return std::string("unknownComment");
    }
}

std::string BiliBiliCommentContainer::getStrByPool(BiliBiliCommentContainer::BilibiliCommentPool _pool){
    switch (_pool){
        case pool_normalPool:           return std::string("normalPool");
        case pool_subtitlePool:         return std::string("subtitlePool");
        case pool_unknown:              return std::string("unknownPool");
    }
}

#endif
