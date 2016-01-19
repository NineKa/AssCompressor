#ifndef AssCompressor_assConverter_H
#define AssCompressor_assConverter_H

#include "bilibiliCommentContainer.h"
#include "assUtil.h"
#include "commentPlaceStream.h"
#include "configure.h"
#include "pipeError.h"
#include "rapidxml/rapidxml.hpp"
#include <utility>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <exception>

#ifdef DEBUG
#include <iostream>
#endif

typedef void (*reduceFuncType) (std::vector<bilibiliCommentContainer*>,
                                std::vector<bilibiliCommentContainer*>);

struct assConverterForceType{
    void (*func)(bilibiliCommentContainer*&,assUtil::assConfigure&,void*);
    void* dataCF;
    const std::string what;
    assConverterForceType(void (*_func)(bilibiliCommentContainer*&,assUtil::
                                        assConfigure&,void*),
                          void* _dataCF,
                          const char* _what) :
                            func(_func),dataCF(_dataCF),
                            what(std::string(_what)){};
    assConverterForceType(void (*_func)(bilibiliCommentContainer*&,assUtil::
                                        assConfigure&,void*),
                          void* _dataCF) :
                            assConverterForceType(_func, _dataCF,"ANONYMOUS"){};
};

struct assConverterForceRawType{
    void (*func)(char*, char*, assUtil::assConfigure&, void*);
    void* dataCF;
    const std::string what;
    assConverterForceRawType(void (*_func)(char*, char*,
                            assUtil::assConfigure&,void*),
                             void* _dataCF,
                             const char* _what):
                            func(_func),dataCF(_dataCF),
                            what(std::string(_what)){};
    assConverterForceRawType(void (*_func)(char*, char*,
                                           assUtil::assConfigure&,void*),
                             void* _dataCF):
                          assConverterForceRawType(_func,_dataCF,"ANONYMOUS"){};

};

struct assConverterFXStrType{
    std::string (*func)(bilibiliCommentContainer*,assUtil::assConfigure&,void*);
    void* dataCF;
    const std::string what;
    assConverterFXStrType(std::string (*_func)(bilibiliCommentContainer*,
                          assUtil::assConfigure&,void*),
                          void* _dataCF,
                          const char* _what):
                            func(_func),dataCF(_dataCF),
                            what(std::string(_what)){};
    assConverterFXStrType(std::string (*_func)(bilibiliCommentContainer*,
                                               assUtil::assConfigure&,void*),
                          void* _dataCF):
                            assConverterFXStrType(_func, _dataCF,"ANONYMOUS"){};
};

class assConverter {
    commentPlaceStream* topRollStream = nullptr;
    commentPlaceStream* bottomRollStream = nullptr;
    commentPlaceStream* reverseTopStream = nullptr;
    commentPlaceStream* topFixStream = nullptr;
    commentPlaceStream* bottomFixStream = nullptr;
    
    assUtil::assConfigure configure;
    std::pair<size_t, double> dataCF;
    std::vector<std::pair<std::string,std::string>> log;
    std::string commentXML;
    std::string assContent;
    std::vector<bilibiliCommentContainer*> containerList;
    std::vector<bilibiliCommentContainer*> collectList;
    
    std::vector<assConverterForceRawType> force_raw;
    std::vector<assConverterForceType> force;
    std::vector<assConverterFXStrType> assFXStr;
    std::vector<reduceFuncType> reduce_list;
    
    std::string assHeader;
    
    static double velocityCalFunc(size_t _length, void* _dataCF){
        const size_t _canvasX = ((std::pair<size_t,double>*)_dataCF)->first;
        const double _duration = ((std::pair<size_t,double>*)_dataCF)->second;
        if (_length <= 12*3) {
            return (_canvasX+12*3)/((_canvasX / 524.0)*_duration);
        }else if(_length <= 15*3){
            return (_canvasX+15*3)/((_canvasX / 524.0)*_duration);
        }else{
            return (_canvasX+_length)/((_canvasX / 524.0)*_duration);
        }
    }
    
    #ifdef bilibiliCommentContainer_SupportTraceLine
    static inline double calDistance(std::pair<size_t, size_t>& _pos1,
                                     std::pair<size_t, size_t>& _pos2){
        return sqrt(pow((double)(_pos1.first - _pos2.first), 2.0) +
                    pow((double)(_pos1.second - _pos2.second), 2.0));
    }
    
    static double calSumDistance(std::vector<std::pair<size_t,size_t>>& _arr){
        double cumDistance = 0.0;
        for (std::vector<std::pair<size_t, size_t>>::iterator iter=_arr.begin();
             (iter+1)!=_arr.end(); iter++) {
            std::pair<size_t, size_t>& pos1 = *iter;
            std::pair<size_t, size_t>& pos2 = *(iter+1);
            double sectionDistance = calDistance(pos1, pos2);
            cumDistance += sectionDistance;
        }
        return cumDistance;
    }
    static inline double calAvgVelcity(
                                  std::vector<std::pair<size_t, size_t>>& _arr,
                                  double _duration){
        double totalDistance = calSumDistance(_arr);
        return totalDistance / _duration;
    }
    std::vector<bilibiliCommentContainer> reduce(
                                             bilibiliCommentContainer& _ACC_15){
        std::vector<bilibiliCommentContainer> returnVector;
        assert(_ACC_15.getType() == bilibiliCommentType::FXComment);
        assert(_ACC_15.fxInfo.type == FXCommentType::ACC_15);
        double cumTime= _ACC_15.getTime();
        double movingDuration = _ACC_15.fxInfo.moveDur;
        double movingPause = _ACC_15.fxInfo.movePau;
        double avgVelocity = calAvgVelcity(_ACC_15.fxInfo.traceline,
                                           movingDuration);
        double avgFadeVelocity = (_ACC_15.fxInfo.fadeOut-_ACC_15.fxInfo.fadeIn)/
                                 _ACC_15.fxInfo.duration;
        if (movingPause > 0.0) {
            std::pair<size_t, size_t> originPoint = _ACC_15.fxInfo.traceline[0];
            char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
            if (buffer == nullptr) {
                throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
            }
            sprintf(buffer, assConverter_attributeConstructor,
                    _ACC_15.getTime(),
                    7,                                             /*FXComment*/
                    _ACC_15.getFontSize(),
                    _ACC_15.getColor(),
                    (unsigned long)_ACC_15.getTimestamp(),
                    (_ACC_15.getPool() == bilibiliCommentPool::SubtitlePool?
                                                                         1 : 0),
                    _ACC_15.getUserID().c_str(),
                    _ACC_15.getCommentID());
            std::string attributeString(buffer);
            memset(buffer, '\0', sizeof(char)*assConverter_MAXBuffer);
            sprintf(buffer, assConverter_ACC_14Constructor,
                    originPoint.first,
                    originPoint.second,
                    _ACC_15.fxInfo.fadeIn,
                    _ACC_15.fxInfo.fadeIn + avgFadeVelocity * movingPause,
                    movingPause,
                    _ACC_15.getContent().c_str(),
                    (int)_ACC_15.fxInfo.zRotate,
                    (int)_ACC_15.fxInfo.yRotate,
                    originPoint.first,
                    originPoint.second,
                    (unsigned long)(movingPause * 1000),
                    0L,
                    (_ACC_15.fxInfo.highlight ? "true" : "false"),
                    _ACC_15.fxInfo.fontname.c_str());
            std::string dataString(buffer);
            free(buffer);
            cumTime += movingPause;
            returnVector.push_back(
                    bilibiliCommentContainer(attributeString,
                                             dataString,
                                             (unsigned int) configure.playerX,
                                             (unsigned int) configure.playerY));
        }
        for (std::vector<std::pair<size_t, size_t>>::iterator iter =
                                               _ACC_15.fxInfo.traceline.begin();
             (iter+1)!=_ACC_15.fxInfo.traceline.end(); iter++) {
            std::pair<size_t,size_t>& pos1 = *iter;
            std::pair<size_t,size_t>& pos2 = *(iter+1);
            double distance = calDistance(pos1, pos2);
            double time = distance / avgVelocity;
            char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
            if (buffer == nullptr) {
                throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
            }
            sprintf(buffer, assConverter_attributeConstructor,
                    cumTime,
                    7,                                             /*FXComment*/
                    _ACC_15.getFontSize(),
                    _ACC_15.getColor(),
                    (unsigned long)_ACC_15.getTimestamp(),
                    (_ACC_15.getPool() == bilibiliCommentPool::SubtitlePool?
                     1 : 0),
                    _ACC_15.getUserID().c_str(),
                    _ACC_15.getCommentID());
            std::string attributeString(buffer);
            memset(buffer, '\0', sizeof(char)*assConverter_MAXBuffer);
            sprintf(buffer, assConverter_ACC_14Constructor,
                    pos1.first,
                    pos1.second,
                    _ACC_15.fxInfo.fadeIn +
                                (cumTime - _ACC_15.getTime()) * avgFadeVelocity,
                    _ACC_15.fxInfo.fadeIn +
                       (cumTime + time + - _ACC_15.getTime()) * avgFadeVelocity,
                    time,
                    _ACC_15.getContent().c_str(),
                    (int)_ACC_15.fxInfo.zRotate,
                    (int)_ACC_15.fxInfo.yRotate,
                    pos2.first,
                    pos2.second,
                    (unsigned long)(time * 1000),
                    0L,
                    (_ACC_15.fxInfo.highlight ? "true" : "false"),
                    _ACC_15.fxInfo.fontname.c_str());
            std::string dataString(buffer);
            free(buffer);
            cumTime += time;
            returnVector.push_back(
                   bilibiliCommentContainer(attributeString,
                                            dataString,
                                            (unsigned int) configure.playerX,
                                            (unsigned int) configure.playerY));
        }
        if (movingPause + movingDuration < _ACC_15.fxInfo.duration){
            std::pair<size_t, size_t> lastPoint = _ACC_15.fxInfo.traceline
                                            [_ACC_15.fxInfo.traceline.size()-1];
            char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
            if (buffer == nullptr) {
                throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
            }
            sprintf(buffer, assConverter_attributeConstructor,
                    cumTime,
                    7,                                             /*FXComment*/
                    _ACC_15.getFontSize(),
                    _ACC_15.getColor(),
                    (unsigned long)_ACC_15.getTimestamp(),
                    (_ACC_15.getPool() == bilibiliCommentPool::SubtitlePool?
                     1 : 0),
                    _ACC_15.getUserID().c_str(),
                    _ACC_15.getCommentID());
            std::string attributeString(buffer);
            memset(buffer, '\0', sizeof(char)*assConverter_MAXBuffer);
            sprintf(buffer, assConverter_ACC_14Constructor,
                    lastPoint.first,
                    lastPoint.second,
                    _ACC_15.fxInfo.fadeIn +
                              (cumTime - _ACC_15.getTime())*avgFadeVelocity,
                    _ACC_15.fxInfo.fadeOut,
                    _ACC_15.fxInfo.duration - movingDuration - movingPause,
                    _ACC_15.getContent().c_str(),
                    (int)_ACC_15.fxInfo.zRotate,
                    (int)_ACC_15.fxInfo.yRotate,
                    lastPoint.first,
                    lastPoint.second,
                    (unsigned long)((_ACC_15.fxInfo.duration -
                                     movingDuration - movingPause) * 1000),
                    0L,
                    (_ACC_15.fxInfo.highlight ? "true" : "false"),
                    _ACC_15.fxInfo.fontname.c_str());
            std::string dataString(buffer);
            free(buffer);
            returnVector.push_back(
                   bilibiliCommentContainer(attributeString,
                                            dataString,
                                            (unsigned int) configure.playerX,
                                            (unsigned int) configure.playerY));
        }
        return returnVector;
    }
    #endif

    void errorLog(rapidxml::xml_node<>* _c, std::exception& _e){
        std::string errPos = std::string("[")+
                             std::string(_c->first_attribute("p")->value())+
                             std::string(",")+
                             std::string(_c->first_node()->value())+
                             std::string("]");
        std::string errMsg = std::string(_e.what());
        log.push_back(std::pair<std::string,std::string>(errPos, errMsg));
    }
    
    int layerFunc(bilibiliCommentContainer* _c){
        int max = configure.layerRollType;
        max = (configure.layerFixType > max)?configure.layerFixType : max;
        max = (configure.layerACCType > max)?configure.layerACCType : max;
        max ++;
        if (configure.setSubtitileOnTop &&
            _c->getPool() == bilibiliCommentPool::SubtitlePool) {
            return max;
        }
        switch (_c -> getType()) {
            case bilibiliCommentType::BottomFixComment:
            case bilibiliCommentType::TopFixComment:
                return configure.layerFixType;
            case bilibiliCommentType::BottomRollComment:
            case bilibiliCommentType::TopRollComment:
            case bilibiliCommentType::ReverseTopRollComment:
                return configure.layerRollType;
            case bilibiliCommentType::FXComment:
                return configure.layerACCType;
            default:
                return 0;
        }
    }
    
    std::string getAnchor(bilibiliCommentContainer* _c){
        assert(_c != nullptr);
        switch (_c ->getType()) {
            case bilibiliCommentType::BottomFixComment:
                return std::string("\\an2");
            case bilibiliCommentType::TopFixComment:
                return std::string("\\an8");
            case bilibiliCommentType::BottomRollComment:
                return std::string("\\an1");
            case bilibiliCommentType::TopRollComment:
                return std::string("\\an7");
            case bilibiliCommentType::ReverseTopRollComment:
                return std::string("\\an9");
            case bilibiliCommentType::FXComment:
                return std::string("\\an7");
            default:
                return std::string("");
        }
    }
    int fontSizeZoom(int _current){
        return (int)(((double)_current/25.0) * configure.defaultFontSize);
    }
    size_t getCollisionHeightN(bilibiliCommentContainer* _c){
        assert(_c != nullptr);
        if (!configure.multiLineCollision) return
                                                fontSizeZoom(_c->getFontSize());
        std::string content = _c->getContent();
        std::string delimiter("/n");
        size_t pos = 0;
        std::string token;
        size_t lineCounter = 0;
        while ((pos = content.find(delimiter)) != std::string::npos) {
            token = content.substr(0, pos);
            lineCounter++;
            content.erase(0, pos + delimiter.length());
        }
        return (++lineCounter) * fontSizeZoom(_c->getFontSize());
    }
    void getMultiLinedContent(std::string& _c){
        if (!configure.multiLineComment) return;
        const std::string search_str="/n";
        const std::string replace_str="\\N";
        long pos=_c.find(search_str);
        while (pos!=-1){
            _c.replace(pos,search_str.length(),replace_str);
            pos=_c.find(search_str);
        }
    }
    size_t getCollisionLengthN(bilibiliCommentContainer* _c){
        assert(_c != nullptr);
        if (!configure.multiLineCollision){
            return assUtil::acturalLength(_c->getContent().c_str()) *
            fontSizeZoom(_c->getFontSize());
        }
        std::vector<std::string> tempVector;
        std::string content = _c->getContent();
        std::string delimiter("/n");
        size_t pos = 0;
        while ((pos = content.find(delimiter)) != std::string::npos) {
            tempVector.push_back(content.substr(0, pos));
            content.erase(0, pos + delimiter.length());
        }
        tempVector.push_back(content);
        size_t length = assUtil::acturalLength(tempVector[0].c_str());
        for (std::vector<std::string>::iterator iter = tempVector.begin();
             iter!=tempVector.end(); iter++){
            if (assUtil::acturalLength((*iter).c_str()) > length){
                length = assUtil::acturalLength((*iter).c_str());
            }
        }
        return length * fontSizeZoom(_c->getFontSize());
    }
    
    std::string getPos_TOPFIX(bilibiliCommentContainer* _c){
        assert(_c != nullptr);
        double time = _c->getTime();
        size_t collisionLength = getCollisionLengthN(_c);
        size_t collisionHeight = getCollisionHeightN(_c);
        size_t select = topFixStream->fetch(time,
                                            collisionLength, collisionHeight);
        char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
        if (buffer == nullptr){
            throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
        }
        sprintf(buffer, "\\pos(%lu,%lu)",
                (unsigned long)(configure.canvasX / 2),
                (unsigned long)select);
        std::string returnStr(buffer);
        free(buffer);
        return returnStr;
    }
    std::string getPos_TOPROLL(bilibiliCommentContainer* _c){
        assert(_c != nullptr);
        double time = _c->getTime();
        size_t collisionLength = getCollisionLengthN(_c);
        size_t collisionHeight = getCollisionHeightN(_c);
        size_t select = topRollStream->fetch(time,
                                             collisionLength, collisionHeight);
        char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
        if (buffer == nullptr){
            throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
        }
        sprintf(buffer, "\\move(%lu,%lu,%ld,%lu)",
                configure.canvasX,
                (unsigned long)select,
                -1*(long)collisionLength,
                (unsigned long)select);
        std::string returnStr(buffer);
        free(buffer);
        return returnStr;
    }
    std::string getPos_BOTTOMFIX(bilibiliCommentContainer* _c){
        assert(_c != nullptr);
        double time = _c->getTime();
        size_t collisionLength = getCollisionLengthN(_c);
        size_t collisionHeight = getCollisionHeightN(_c);
        size_t select = bottomFixStream->fetch(time,
                                              collisionLength, collisionHeight);
        assert(configure.canvasY >= select);
        select = configure.canvasY - select;
        char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
        if (buffer == nullptr){
            throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
        }
        sprintf(buffer, "\\pos(%lu,%lu)",
                (unsigned long)(configure.canvasX / 2),
                (unsigned long)select);
        std::string returnStr(buffer);
        free(buffer);
        return returnStr;
    }
    std::string getPos_BOTTOMROW(bilibiliCommentContainer* _c){
        assert(_c != nullptr);
        double time = _c->getTime();
        size_t collisionLength = getCollisionLengthN(_c);
        size_t collisionHeight = getCollisionHeightN(_c);
        size_t select = bottomRollStream->fetch(time,
                                              collisionLength, collisionHeight);
        assert(configure.canvasY >= select);
        select = configure.canvasY - select;
        char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
        if (buffer == nullptr){
            throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
        }
        sprintf(buffer, "\\move(%lu,%lu,%ld,%lu)",
                configure.canvasX,
                (unsigned long)select,
                -1*(long)collisionLength,
                (unsigned long)select);
        std::string returnStr(buffer);
        free(buffer);
        return returnStr;
    }
    std::string getPos_REVERSETOP(bilibiliCommentContainer* _c){
        assert(_c != nullptr);
        double time = _c->getTime();
        size_t collisionLength = getCollisionLengthN(_c);
        size_t collisionHeight = getCollisionHeightN(_c);
        size_t select = reverseTopStream->fetch(time,
                                              collisionLength, collisionHeight);
        char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
        if (buffer == nullptr){
            throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
        }
        sprintf(buffer, "\\move(%lu,%lu,%lu,%lu)",
                0L,
                (unsigned long)select,
                (unsigned long)(collisionLength+configure.canvasX),
                (unsigned long)select);
        std::string returnStr(buffer);
        free(buffer);
        return returnStr;
    }
    std::string getPos_ACC(bilibiliCommentContainer* _c){
        assert(_c != nullptr);
        assert(_c->getType() == bilibiliCommentType::FXComment);
        switch (_c->fxInfo.type) {
            case FXCommentType::ACC_05:
            case FXCommentType::ACC_07:{
                std::pair<long,long> pos = assUtil::ACCPosAdjust(
                                        _c->fxInfo.start_X, _c->fxInfo.start_Y,
                                        configure.vidRatioX,configure.vidRatioY,
                                        configure.playerX,  configure.playerY,
                                        configure.canvasX,  configure.canvasY);
                char* buffer=(char*)malloc(sizeof(char)*assConverter_MAXBuffer);
                if (buffer == nullptr){
                    throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
                }
                sprintf(buffer, "\\pos(%ld,%ld)",
                        pos.first,
                        pos.second);
                std::string returnStr(buffer);
                free(buffer);
                return returnStr;
            }
            case FXCommentType::ACC_11:
            case FXCommentType::ACC_13:
            case FXCommentType::ACC_14:{
                std::pair<long,long> startPos = assUtil::ACCPosAdjust(
                                        _c->fxInfo.start_X, _c->fxInfo.start_Y,
                                        configure.vidRatioX,configure.vidRatioY,
                                        configure.playerX,  configure.playerY,
                                        configure.canvasX,  configure.canvasY);
                std::pair<long,long> toPos = assUtil::ACCPosAdjust(
                                        _c->fxInfo.to_X,    _c->fxInfo.to_Y,
                                        configure.vidRatioX,configure.vidRatioY,
                                        configure.playerX,  configure.playerY,
                                        configure.canvasX,  configure.canvasY);
                double movingDuration = _c->fxInfo.moveDur;
                double movingPause    = _c->fxInfo.movePau;
                char* buffer=(char*)malloc(sizeof(char)*assConverter_MAXBuffer);
                if (buffer == nullptr){
                    throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
                }
                if (startPos.first == toPos.first &&
                    startPos.second == toPos.second){
                    sprintf(buffer, "\\pos(%ld,%ld)",
                                               startPos.first, startPos.second);
                    std::string returnStr(buffer);
                    free(buffer);
                    return returnStr;
                }
                sprintf(buffer, "\\move(%ld,%ld,%ld,%ld,%lu,%lu)",
                       startPos.first,
                       startPos.second,
                       toPos.first,
                       toPos.second,
                       (unsigned long)floor(movingPause * 1000),
                       (unsigned long)floor((movingDuration+movingPause)*1000));
                std::string returnStr(buffer);
                free(buffer);
                return returnStr;
            }
            case FXCommentType::ACC_15:
            case FXCommentType::Unknown_Format:{
                return std::string("");
            }
        }
    }
    std::string getPos(bilibiliCommentContainer* _c){
        switch (_c -> getType()) {
            case bilibiliCommentType::TopRollComment:
                return getPos_TOPROLL(_c);
            case bilibiliCommentType::BottomRollComment:
                return getPos_BOTTOMROW(_c);
            case bilibiliCommentType::BottomFixComment:
                return getPos_BOTTOMFIX(_c);
            case bilibiliCommentType::TopFixComment:
                return getPos_TOPFIX(_c);
            case bilibiliCommentType::ReverseTopRollComment:
                return getPos_REVERSETOP(_c);
            case bilibiliCommentType::FXComment:
                return getPos_ACC(_c);
            default:
                return std::string("");
        }
    }

    std::string generateForceList(){
        if (force.empty()) return std::string("[]");
        std::string returnStr = "[";
        for (std::vector<assConverterForceType>::iterator iter = force.begin();
             iter != force.end(); iter++) {
            returnStr += iter->what + ",";
        }
        returnStr = returnStr.substr(0, returnStr.length()-1) + "]";
        return returnStr;
    }
    std::string generateForceRawList(){
        if (force_raw.empty()) return std::string("[]");
        std::string returnStr = "[";
        typedef std::vector<assConverterForceRawType>::iterator iterType;
        for (iterType iter = force_raw.begin();iter != force_raw.end();iter++) {
            returnStr += iter->what + ",";
        }
        returnStr = returnStr.substr(0, returnStr.length()-1) + "]";
        return returnStr;
    }
    std::string generateFXStrList(){
        if (assFXStr.empty()) return std::string("[]");
        std::string returnStr = "[";
        typedef std::vector<assConverterFXStrType>::iterator iterType;
        for (iterType iter = assFXStr.begin();iter != assFXStr.end();iter++) {
            returnStr += iter->what + ",";
        }
        returnStr = returnStr.substr(0, returnStr.length()-1) + "]";
        return returnStr;
    }
public:
    std::vector<std::pair<std::string,std::string>> fetchLog() {return log;}
    static double durationDecisionFunc(bilibiliCommentContainer* _c,
                                       assUtil::assConfigure& _configure){
        switch (_c->getType()) {
            case bilibiliCommentType::TopRollComment:
            case bilibiliCommentType::BottomRollComment:
            case bilibiliCommentType::ReverseTopRollComment:
                return (_configure.canvasX / 524.0) * _configure.duration;
            case bilibiliCommentType::BottomFixComment:
            case bilibiliCommentType::TopFixComment:
                return _configure.duration;
            case bilibiliCommentType::FXComment:
                return _c->fxInfo.duration;
            case bilibiliCommentType::CodeComment:
            case bilibiliCommentType::Unknown_Comment:
                return 0.0;
        }
    }
    
    void queue_force_raw(const assConverterForceRawType _forceRawType){
        force_raw.push_back(_forceRawType);
    }
    void queue_force(const assConverterForceType _forceType){
        force.push_back(_forceType);
    }
    void queue_FXStr(const assConverterFXStrType _fxStrType){
        assFXStr.push_back(_fxStrType);
    }
    void queue_reduce(const reduceFuncType _func){
        this->reduce_list.push_back(_func);
    }
    
    void forceReduce(void (*_func)(std::vector<bilibiliCommentContainer*>,std::vector<bilibiliCommentContainer*>)){
        _func(containerList, collectList);
    }
    
    assConverter(assUtil::assConfigure& _config){
        configure = _config;
        dataCF = std::pair<size_t, double>(configure.canvasX,
                                           configure.duration);
        topRollStream = new commentPlaceStream(configure.canvasX,
                                               configure.canvasY,
                                               velocityCalFunc,
                                               (void*)&dataCF);
        bottomRollStream = new commentPlaceStream(configure.canvasX,
                                                  configure.canvasY,
                                                  velocityCalFunc,
                                                  (void*)&dataCF);
        reverseTopStream = new commentPlaceStream(configure.canvasX,
                                                  configure.canvasY,
                                                  velocityCalFunc,
                                                  (void*)&dataCF);
        topFixStream = new commentPlaceStream(0,configure.canvasY,
                                             assUtil::defaultStaticVelocityFunc,
                                             (void*)&dataCF.second);
        bottomFixStream = new commentPlaceStream(0,configure.canvasY,
                                             assUtil::defaultStaticVelocityFunc,
                                             (void*)&dataCF.second);
    }
    
    static bool compareFunc(bilibiliCommentContainer* _a,
                   bilibiliCommentContainer* _b){
        return _a->getTime() < _b->getTime();
    }
    
    std::vector<bilibiliCommentContainer*> getContainerList(){
        return this->containerList;
    }
    
    void analysis(std::string& _data){
        rapidxml::xml_document<> xmlHandler;
        char* buffer = (char*)malloc(sizeof(char)*(_data.length()+1));
        strcpy(buffer, _data.c_str());
        if (buffer == nullptr) {
            throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
        }
        try {
            xmlHandler.parse<0>(buffer);
        } catch (...){
            throw std::runtime_error(ASSCONVERTER_BAD_XML);
        }
        rapidxml::xml_node<>* rootnode = xmlHandler.first_node();
        for (rapidxml::xml_node<>* iter = rootnode->first_node("d");
             iter != nullptr; iter = iter->next_sibling()) {
            if (strcmp(iter->name(), "d") != 0) continue;
            try {
                char* a = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
                char* d = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
                if (a == nullptr || d == nullptr) {
                    throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
                }
                if (iter->first_attribute("p") == nullptr ||
                    iter->first_node() == nullptr) continue;
                strcpy(a, iter->first_attribute("p")->value());
                strcpy(d, iter->first_node()->value());
                /*apply assConverterForceRawType list*/
                for (std::vector<assConverterForceRawType>::iterator iter =
                     force_raw.begin(); iter!=force_raw.end(); iter++) {
                    void* dataCF = iter->dataCF;
                    void (*func)(char*, char*, assUtil::assConfigure&,void*)
                                                                   = iter->func;
                    func(a,d,configure,dataCF);
                }
                std::string attributeStr(a);
                std::string dataStr(d);
                free(a); free(d);
                bilibiliCommentContainer* _c = new bilibiliCommentContainer
                                            (attributeStr,
                                            dataStr,
                                            (unsigned int)configure.playerX,
                                            (unsigned int)configure.playerY);
                #ifndef bilibiliCommentContainer_SupportTraceLine
                containerList.push_back(_c);
                #else
                if (_c->getType() == bilibiliCommentType::FXComment &&
                    _c->fxInfo.type == FXCommentType::ACC_15) {
                    std::vector<bilibiliCommentContainer> reduceList =
                                                                    reduce(*_c);
                    for (std::vector<bilibiliCommentContainer>::iterator iter =
                         reduceList.begin(); iter!=reduceList.end(); iter++) {
                        bilibiliCommentContainer* append =
                                            new bilibiliCommentContainer(*iter);
                        containerList.push_back(append);
                    }
                }else{
                    containerList.push_back(_c);
                }
                #endif
            } catch (std::exception& err) {
                errorLog(iter, err);
                continue;
            }
        }
        xmlHandler.clear();
        free(buffer);
        std::stable_sort(containerList.begin(),
                         containerList.end(),
                         assConverter::compareFunc);
        collectList = containerList;
        /*Apply reduceFunc List*/
        for (std::vector<reduceFuncType>::iterator iter = reduce_list.begin() ;
             iter != reduce_list.end(); iter++){
            (*iter)(containerList, collectList);
            std::stable_sort(containerList.begin(),
                             containerList.end(),
                             assConverter::compareFunc);
            containerList.erase(std::remove(containerList.begin(),
                                            containerList.end(), nullptr),
                                containerList.end());
        }
        /*Apply assConverterForceType List*/
        for (std::vector<assConverterForceType>::iterator iter =
             force.begin(); iter!=force.end(); iter++) {
            void (*func)(bilibiliCommentContainer*&,assUtil::assConfigure&,
                                                            void*) = iter->func;
            void* dataCF = iter->dataCF;
            for (std::vector<bilibiliCommentContainer*>::iterator iter2 =
                 containerList.begin(); iter2!=containerList.end(); iter2++) {
                func(*iter2, configure,dataCF);
            }
        }
        containerList.erase(std::remove(containerList.begin(),
                                        containerList.end(), nullptr),
                            containerList.end());
    }
    
    void setHeader(std::string& _header){assHeader = _header;}
    
    std::string assFile(){
        std::string assFileData;
        char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
        if (buffer == nullptr) {
            throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
        }
        sprintf(buffer,
                "[Script Info] \n"
                "%s"
                "Title: %s \n"
                "Original Script: %s \n"
                "ScriptType: v4.00+ \n"
                "PlayResX: %lu \n"
                "PlayResY: %lu \n\n\n"
                ,assHeader.c_str(), configure.bilibiliTitle.c_str(),
                configure.bilibiliCommentUrl.c_str(),
                (unsigned long)configure.canvasX,
                (unsigned long)configure.canvasY);
        assFileData = std::string(buffer);
        sprintf(buffer,
                "[V4+ Styles] \n"
                "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding \n"
                "Style: Default,%s,%lu,&H00FFFFFF,&H00FFFFFF,&H00000000,&H00000000,0,0,0,0,100,100,0,0,1,2,0,2,20,20,2,0\n\n\n"
                , configure.defaultFontName.c_str(),
                (unsigned long)configure.defaultFontSize);
        assFileData += std::string(buffer);
        assFileData += std::string(
        "[Events]\n"
        "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
                                   );
        for (std::vector<bilibiliCommentContainer*>::iterator iter =
             containerList.begin(); iter!=containerList.end(); iter++) {
            std::string userFXStr = "";
            for (std::vector<assConverterFXStrType>::iterator iter_func =
                 assFXStr.begin(); iter_func!=assFXStr.end(); iter_func++){
                std::string (*func)(bilibiliCommentContainer*,
                                    assUtil::assConfigure&,
                                    void*) = (*iter_func).func;
                void* data = (*iter_func).dataCF;
                userFXStr += func(*iter, configure, data);
            }
            std::string fxStr= getAnchor(*iter)+userFXStr+getPos(*iter);
            std::string content((*iter)->getContent());
            if (configure.multiLineComment) getMultiLinedContent(content);
            sprintf(buffer,
                    "Dialogue: %d,%s,%s,Default,,20,20,2,,{%s}%s\n",
                    layerFunc(*iter),
                    assUtil::assTime((*iter)->getTime()).c_str(),
                    assUtil::assTime((*iter)->getTime()+
                                durationDecisionFunc(*iter, configure)).c_str(),
                    fxStr.c_str(),
                    content.c_str());
            assFileData+=std::string(buffer);
        }
        #ifdef assConverter_Append_Statistic
        sprintf(buffer, ";#Comment : %lu \n"
                        ";#Input   : %lu \n"
                        ";#ForceType_Raw : %lu \n"
                        ";#ForceType     : %lu \n"
                        ";#FXForceType   : %lu \n"
                        ";#ReduceFunc    : %lu \n"
                        ";Version  : %s \n",
                (unsigned long)containerList.size(),
                (unsigned long)collectList.size(),
                (unsigned long)force_raw.size(),
                (unsigned long)force.size(),
                (unsigned long)assFXStr.size(),
                (unsigned long)reduce_list.size(),
                ASSCOMPRESSOR_Signature);
        assFileData += std::string(buffer);
        free(buffer);
        #endif
        #ifdef assConverter_Append_ForceList
        assFileData +=";ForceRawType : " + generateForceRawList() + "\n";
        assFileData +=";ForceType    : " + generateForceList() + "\n";
        assFileData +=";FXStrType    : " + generateFXStrList() + "\n";
        #endif
        return assFileData;
    }
    
    ~assConverter(){
        delete topRollStream;
        delete bottomRollStream;
        delete reverseTopStream;
        delete topFixStream;
        delete bottomFixStream;
        for (std::vector<bilibiliCommentContainer*>::iterator iter =
                     collectList.begin(); iter!=collectList.end(); iter++) {
            delete *iter;
        }
    }
};
#endif