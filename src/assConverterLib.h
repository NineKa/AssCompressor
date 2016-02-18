#ifndef AssCompressor_assConverterForceFuncs_H
#define AssCompressor_assConverterForceFuncs_H
#include "assConverter.h"
#include "bilibiliCommentContainer.h"
#include "assUtil.h"
#include "configure.h"
#include <limits>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>

#ifdef CXX11REGEX
    #include <regex>
#else
    #include <boost/regex.h>
#endif
namespace assConverterLib{
    namespace implementation{
        void removeVisitorCommentFunc(bilibiliCommentContainer*& _container,
                                      assUtil::assConfigure&,
                                      void*){
            if (_container == nullptr) return;
            const char* userID= _container->getUserID().c_str();
            if (userID[0] == 'D') _container = nullptr;
            return;
        }
        template <bilibiliCommentType _type> void remove
         (bilibiliCommentContainer*& _container, assUtil::assConfigure&, void*){
              if (_container == nullptr) return;
              if (_container->getType() == _type) _container = nullptr;
        }
        size_t findFirst(const char* _str, char _mark){
            for (size_t iter = 0; _str[iter] != '\0'; iter++) {
                if (_str[iter] == _mark) return iter;
            }
            return std::numeric_limits<size_t>::max();
        }
        size_t countMark(const char* _str, char _mark){
            size_t time = 0;
            bool ignore = false;
            for (size_t iter = 0; iter < strlen(_str); iter++){
                ignore = (_str[iter] == '\"')? !ignore : ignore;
                if (ignore) continue;
                time += (_str[iter] == _mark)? 1L : 0L;
            }
            return time;
        }
        void fixIncompleteFXContent(char* _a, char* _d,
                                    assUtil::assConfigure&,
                                    void*){
            assert(_a != nullptr && _d != nullptr);
            size_t firstComma = findFirst(_a, ',');
            char&  typeInt = _a[firstComma+1];
            if (typeInt != '7') return;
            if (countMark(_d, ',')<14) return;
            if (_d[strlen(_d)-1] == ']' && _d[strlen(_d)-2] == '\"') return;
            strcat(_d, "\"]");
            return;
        }
        int fadeCalFunc(bilibiliCommentContainer* _c,
                        assUtil::assConfigure& _configure){
            switch (_c->getType()) {
                case bilibiliCommentType::TopRollComment:
                    return 255 - (int)floor(_configure.fadeConfigArray[1]*255);
                case bilibiliCommentType::BottomRollComment:
                    return 255 - (int)floor(_configure.fadeConfigArray[2]*255);
                case bilibiliCommentType::BottomFixComment:
                    return 255 - (int)floor(_configure.fadeConfigArray[4]*255);
                case bilibiliCommentType::TopFixComment:
                    return 255 - (int)floor(_configure.fadeConfigArray[5]*255);
                case bilibiliCommentType::ReverseTopRollComment:
                    return 255 - (int)floor(_configure.fadeConfigArray[6]*255);
                case bilibiliCommentType::FXComment:
                    return 255 - (int)floor(_configure.fadeConfigArray[7]*255);
                case bilibiliCommentType::CodeComment:
                    return 255 - (int)floor(_configure.fadeConfigArray[8]*255);
                case bilibiliCommentType::Unknown_Comment:
                    return 255 - (int)floor(_configure.fadeConfigArray[0]*255);
            }
        }
        std::string getColorStrN(bilibiliCommentContainer* _c,
                                 assUtil::assConfigure&, void*){
            if (_c->getColor() == 0xffffff) {return std::string("");}
            char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
            if (buffer == nullptr){
                throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
            }
            sprintf(buffer,"\\1c&H%s&",
                    assUtil::assColor(_c->getColor()).c_str());
            std::string fillColor(buffer);
            free(buffer);
            std::string borderColor =
            (strcmp(assUtil::assColor(_c->getColor()).c_str(), "000000")==0)?
            std::string("\\3c&Hffffff&"):std::string("\\3c&H000000&");
            if (_c->getType() == bilibiliCommentType::FXComment &&
                (_c->fxInfo.type == FXCommentType::ACC_14 ||
                 _c->fxInfo.type == FXCommentType::ACC_15)&&
                _c->fxInfo.highlight == false) {
                return fillColor;
            }
            return fillColor+borderColor;
        }
        std::string fontSizeStr(bilibiliCommentContainer* _c,
                                assUtil::assConfigure& _configure, void*){
            char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
            if (buffer == nullptr){
                throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
            }
            int fontsize;
            if (_c->getType() == bilibiliCommentType::FXComment) {
                fontsize = assUtil::ACCFontSizeAdjust(_c->getFontSize(),
                                     _configure.vidRatioX, _configure.vidRatioY,
                                     _configure.playerX, _configure.playerY,
                                     _configure.canvasX, _configure.canvasY);
            }else{
                fontsize = (int)(((double)(_c->getFontSize())/25.0)
                                 * _configure.defaultFontSize);
            }
            if ((size_t)fontsize == _configure.defaultFontSize) {
                free(buffer);
                return std::string("");
            }
            sprintf(buffer, "\\fs%d", fontsize);
            std::string returnStr(buffer);
            free(buffer);
            return returnStr;
        }
        std::string fadeStr(bilibiliCommentContainer* _c,
                            assUtil::assConfigure& _configure, void*){
            char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
            if (buffer == nullptr){
                throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
            }
            if (_c->getType() == bilibiliCommentType::FXComment) {
                double ratio = _configure.fadeConfigArray[7];
                if (_c->fxInfo.fadeIn == _c->fxInfo.fadeOut){
					if (_c->fxInfo.fadeIn == 0.0){
						free(buffer);
						return std::string("");
					}
                    sprintf(buffer, "\\alpha&H%02x",
                            255 - (int)(_c->fxInfo.fadeIn * ratio * 255));
                    std::string returnStr(buffer);
                    free(buffer);
                    return returnStr;
                }
                sprintf(buffer, "\\fade(%s,%s,%s,%lu,%lu,%lu,%lu)",
                        assUtil::assFade(_c->fxInfo.fadeIn  * ratio).c_str(),
                        assUtil::assFade(_c->fxInfo.fadeOut * ratio).c_str(),
                        assUtil::assFade(_c->fxInfo.fadeOut * ratio).c_str(),
                        0L,
                        (unsigned long)floor(_c->fxInfo.duration * 1000),
                        (unsigned long)floor(_c->fxInfo.duration * 1000),
                        (unsigned long)floor(_c->fxInfo.duration * 1000));
                std::string returnStr(buffer);
                free(buffer);
                return returnStr;
            }else{
				if (fadeCalFunc(_c, _configure) == 0.0) {
					free(buffer);
					return std::string("");
				}
                sprintf(buffer, "\\alpha&H%02x", fadeCalFunc(_c, _configure));
                std::string returnStr(buffer);
                free(buffer);
                return returnStr;
            }
        }
        std::string rotateStr(bilibiliCommentContainer* _c,
                              assUtil::assConfigure&, void*){
            if (_c->getType() != bilibiliCommentType::FXComment) {
                return std::string("");
            }
            if (_c->fxInfo.zRotate == 0 && _c->fxInfo.yRotate == 0){
                return std::string("");
            }
            char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
            if (buffer == nullptr){
                throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
            }
            sprintf(buffer, "\\fry%d", -(int)_c->fxInfo.yRotate);
            std::string yRotateStr(buffer);
            sprintf(buffer, "\\frz%d", -(int)_c->fxInfo.zRotate);
            std::string zRotateStr(buffer);
            free(buffer);
            return yRotateStr + zRotateStr;
        }
        std::string borderStr(bilibiliCommentContainer* _c,
                              assUtil::assConfigure& _configure, void*){
            char* buffer = (char*)malloc(sizeof(char)*assConverter_MAXBuffer);
            if (buffer == nullptr){
                throw std::runtime_error(ASSCONVERTER_NOT_ENGOUTH_ROOM);
            }
            if (_c->getType() == bilibiliCommentType::FXComment &&
                (_c->fxInfo.type == FXCommentType::ACC_14 ||
                 _c->fxInfo.type == FXCommentType::ACC_15) &&
                _c->fxInfo.highlight == false){
                free(buffer);
                return std::string("\\bord0");
            }
            sprintf(buffer, "\\bord%.2f", _configure.borderSize);
            std::string returnStr(buffer);
            free(buffer);
            return returnStr;
        }
        std::string fxFontNameStrNormal(bilibiliCommentContainer* _c,
                              assUtil::assConfigure&, void*){
            if (_c ->getType() != bilibiliCommentType ::FXComment){
                return std::string("");
            }
            if (_c ->fxInfo.type == FXCommentType::ACC_05 ||
                _c ->fxInfo.type == FXCommentType::ACC_07 ||
                _c ->fxInfo.type == FXCommentType::ACC_11){
                return std::string("");
            }
            return std::string("\\fn")+_c->fxInfo.fontname;
        }
        std::string fxFontNameStrMac(bilibiliCommentContainer* _c,
                                     assUtil::assConfigure&, void*){
            if (_c ->getType() != bilibiliCommentType ::FXComment){
                return std::string("");
            }
            if (_c ->fxInfo.type == FXCommentType::ACC_05 ||
                _c ->fxInfo.type == FXCommentType::ACC_07 ||
                _c ->fxInfo.type == FXCommentType::ACC_11){
                return std::string("");
            }
            std::vector<std::pair<const char*, const char*>> tempCompare;
            tempCompare.push_back(std::pair<const char*, const char*>(
                                  BilibiliFont_KEY_HEI,
                                  BilibiliFont_Name_HEI));
            tempCompare.push_back(std::pair<const char*, const char*>(
                                  BilibiliFont_KEY_Kai,
                                  BilibiliFont_Name_Kai));
            tempCompare.push_back(std::pair<const char*, const char*>(
                                  BilibiliFont_KEY_MS_HEI,
                                  BilibiliFont_Name_MS_HEI));
            tempCompare.push_back(std::pair<const char*, const char*>(
                                  BilibiliFont_KEY_Song,
                                  BilibiliFont_Name_Song));
            tempCompare.push_back(std::pair<const char*, const char*>(
                                  BilibiliFont_KEY_YouYuan,
                                  BilibiliFont_Name_YouYuan));
            for (std::vector<std::pair<const char*, const char*>>::iterator
                 iter = tempCompare.begin(); iter!=tempCompare.end(); iter++){
                if (strcmp((*iter).first, _c->fxInfo.fontname.c_str()) == 0){
                    return std::string("\\fn")+std::string((*iter).second);
                }
            }
            return std::string("");
        }
        void removeTopRollf
        (bilibiliCommentContainer*& _container, assUtil::assConfigure&, void*){
            if (_container == nullptr) return;
            if (_container->getType() == bilibiliCommentType::TopRollComment) _container = nullptr;
        }
        void removeBottomRollf
        (bilibiliCommentContainer*& _container, assUtil::assConfigure&, void*){
            if (_container == nullptr) return;
            if (_container->getType() == bilibiliCommentType::BottomRollComment) _container = nullptr;
        }
        void removeBottomFixf
        (bilibiliCommentContainer*& _container, assUtil::assConfigure&, void*){
            if (_container == nullptr) return;
            if (_container->getType() == bilibiliCommentType::BottomFixComment) _container = nullptr;
        }
        void removeTopFixf
        (bilibiliCommentContainer*& _container, assUtil::assConfigure&, void*){
            if (_container == nullptr) return;
            if (_container->getType() == bilibiliCommentType::TopFixComment) _container = nullptr;
        }
        void removeReverseTopRollf
        (bilibiliCommentContainer*& _container, assUtil::assConfigure&, void*){
            if (_container == nullptr) return;
            if (_container->getType() == bilibiliCommentType::TopRollComment) _container = nullptr;
        }
        void removeFXf
        (bilibiliCommentContainer*& _container, assUtil::assConfigure&, void*){
            if (_container == nullptr) return;
            if (_container->getType() == bilibiliCommentType::FXComment) _container = nullptr;
        }
        void removeCodef
        (bilibiliCommentContainer*& _container, assUtil::assConfigure&, void*){
            if (_container == nullptr) return;
            if (_container->getType() == bilibiliCommentType::CodeComment) _container = nullptr;
        }
        void removeUnknownf
        (bilibiliCommentContainer*& _container, assUtil::assConfigure&, void*){
            if (_container == nullptr) return;
            if (_container->getType() == bilibiliCommentType::Unknown_Comment) _container = nullptr;
        }
    }
    
    assConverterForceType removeVisitorComment(
                            implementation:: removeVisitorCommentFunc,
                                               nullptr,
                                               "removeVisitorComment");
    
    template <bilibiliCommentType _t> assConverterForceType removeByType(){
        bilibiliCommentType _type = _t;
        std::string modName = bilibiliCommentContainer::commentType2Str(_type);
        modName = "removeType" + modName;
        return assConverterForceType(implementation::remove<_t>,
                                     nullptr,
                                     modName.c_str());
    }
    
    assConverterForceRawType autoFixFXString(implementation::fixIncompleteFXContent, nullptr, "autoFixFXString");
    
    assConverterForceType removeTopRoll(implementation::removeTopRollf,nullptr,"removeTopRoll");
    assConverterForceType removeBottomRoll(implementation::removeBottomFixf,nullptr,"removeBottomRoll");
    assConverterForceType removeBottomFix(implementation::removeBottomFixf,nullptr,"removeBottomFix");
    assConverterForceType removeTopFix(implementation::removeTopFixf,nullptr,"removeTopFix");
    assConverterForceType removeReverseTopRoll(implementation::removeReverseTopRollf,nullptr,"removeReverseTopRoll");
    assConverterForceType removeFX(implementation::removeFXf,nullptr,"removeFX");
    assConverterForceType removeCode(implementation::removeCodef,nullptr,"removeCode");
    assConverterForceType removeUnknown(implementation::removeUnknownf,nullptr,"removeUnknown");
    
    assConverterFXStrType colorComment(implementation::getColorStrN, nullptr, "colorComment");
    assConverterFXStrType fontSizeSpecify(implementation::fontSizeStr, nullptr, "fontSizeSpecify");
    assConverterFXStrType fade(implementation::fadeStr, nullptr, "fade");
    assConverterFXStrType rotation(implementation::rotateStr, nullptr, "rotation");
    assConverterFXStrType borderSpecify(implementation::borderStr, nullptr, "borderSpecify");
    assConverterFXStrType fxFontNameNormal(implementation::fxFontNameStrNormal, nullptr, "fxFontNameNormal");
    assConverterFXStrType fxFontNameMac(implementation::fxFontNameStrMac, nullptr, "fxFontNameMac");
}
#endif