#include "bilibiliWebManager.h"
#include "assConverter.h"
#include "colorize.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "assUtil.h"
#include "netFetcher.h"
#include <iostream>
#include <fstream>
#include <exception>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <dlfcn.h>
#include <boost/filesystem.hpp>
#include <cmath>

#define AssCompressorCLI_NOTENOUGHROOM "AssCompressorCLI Not Enough Room"
#define AssCompressorCLI_BADLOADXML    "AssCompressorCLI Bad Load XML"
#define AssCompressorCLI_BADCONFGURE   "AssCompressorCLI Bad Configure"
#define AssCompressorCLI_BADBUNDLE     "AssCompressorCLI Bad Bundle"
#define AssCompressorCLI_FUNCNOTEXIST  "AssCompressorCLI Func Not Exist"
#define AssCompressorCLI_ROLLBACK      "AssCompressorCLI Error Encoutered during rolling back"
#define AssCompressorCLI_LIBNOTEXIST   "AssCompressorCLI Library Not Exists"

#define AssCompressorCLI_MAX_BUFFER    102400
#define AssCompressorCLI_URL_BUFFER    1024

using namespace std;
using namespace assUtil;

enum forceFuncType{
    converterForce,
    converterForceRaw,
    converterFXStr,
};

std::vector<std::pair<void*,std::string>> dlHanle;
std::vector<assConverterForceType> force_list;
std::vector<assConverterForceRawType> force_raw_list;
std::vector<assConverterFXStrType> fxStr_list;

forceFuncType deduceType(const char* _str){
    char* buffer = (char*)malloc(sizeof(char)*(strlen(_str) + 1));
    if (buffer == nullptr) throw runtime_error(AssCompressorCLI_NOTENOUGHROOM);
    std::string a(_str);
    for (size_t iter = 0; iter < strlen(_str); iter++){
        buffer[iter] = (char)toupper((int)_str[iter]);
    }
    if (strcmp(buffer, "CONVERTERFORCE") == 0) {
        free(buffer);
        return forceFuncType::converterForce;
    } else if (strcmp(buffer, "CONVERTERFORCERAW") == 0) {
        free(buffer);
        return forceFuncType::converterForceRaw;
    } else if (strcmp(buffer, "CONVERTERFXSTR") == 0){
        free(buffer);
        return forceFuncType::converterFXStr;
    } else {
        throw runtime_error(AssCompressorCLI_BADLOADXML);
    }
}
void unload(){
    for (std::vector<std::pair<void*,std::string>>::iterator iter =
         dlHanle.begin(); iter != dlHanle.end(); iter++) dlclose(iter->first);
}
void loadAdditionForceFunc(const char* _loadPath){
    if (!boost::filesystem::exists(_loadPath)) throw runtime_error(AssCompressorCLI_LIBNOTEXIST);
    rapidxml::file<> xmlFile(_loadPath);
    rapidxml::xml_document<> xmlParser;
    xmlParser.parse<0>(xmlFile.data());
    rapidxml::xml_node<>* rootNode = xmlParser.first_node();
    for (rapidxml::xml_node<>* libIter = rootNode->first_node("lib");
         libIter != nullptr;
         libIter = libIter -> next_sibling()) {
        if (libIter == nullptr) break;
        
        if (strcmp(libIter->name(), "lib") !=0 ) continue;
        const char* libPath = libIter->first_node("path")->first_node()->value();
        const char* bundleInfo = libIter->first_node("bundle")->first_node()->value();
        std::string infoStr(bundleInfo);
        
        void* libHandle = dlopen(libPath, RTLD_LAZY);
        if (libHandle == nullptr) {
            throw runtime_error(AssCompressorCLI_BADBUNDLE);
        }
        dlHanle.push_back(std::pair<void*, std::string>(libHandle, infoStr));
        
        for (rapidxml::xml_node<>* funcIter = libIter->first_node("func");
             funcIter != nullptr;
             funcIter = funcIter->next_sibling()) {
            if (strcmp(funcIter->name(), "func") != 0) continue;
            
            forceFuncType funcType = deduceType(funcIter->
                                                first_attribute("type")->
                                                value());
            const char* funcName = funcIter->first_node()->value();
            dlerror();
            void* structPtr = dlsym(libHandle, funcName);
            if (structPtr == nullptr) {
                throw runtime_error(AssCompressorCLI_BADBUNDLE);
            } else {
                switch (funcType) {
                    case converterForce:{
                        force_list.push_back((*((assConverterForceType*)structPtr)));
                        break;
                    }
                    case converterForceRaw:{
                        force_raw_list.push_back(*((assConverterForceRawType*)structPtr));
                        break;
                    }
                    case converterFXStr:{
                        fxStr_list.push_back(*((assConverterFXStrType*)structPtr));
                        break;
                    }
                }
            }
        }
    }
    xmlParser.clear();
}
void usage(){
    std::cout<<ASSCOMPRESSOR_Signature<<" @K9"<<std::endl;
    std::cout<<"AssCompressor converts bilibili XML Comment file into standard ass subtitle file."<<std::endl;
    std::cout<<colorize(std::string("\tAssCompressorCLI [-v] [-l _path] [-i _path] [-o _path] [-stdin]"), ColorIO_Style::Highlight)<<std::endl;
    std::cout<<colorize(std::string("-v"), ColorIO_Style::Highlight)<<'\t'<<"Display the version of AssCompressor"<<std::endl;
    std::cout<<colorize(std::string("-l"), ColorIO_Style::Highlight)<<'\t'<<"Load the library configure XML from _path"<<std::endl;
    std::cout<<colorize(std::string("-i"), ColorIO_Style::Highlight)<<'\t'<<"Load the target configure XML from _path"<<std::endl;
    std::cout<<colorize(std::string("-stdin"), ColorIO_Style::Highlight)<<'\t'<<"Receive target configure from stdin"<<std::endl;
    std::cout<<colorize(std::string("-o"), ColorIO_Style::Highlight)<<'\t'<<"Sepcify target output, default is STDOUT"<<std::endl;
}
void version(){
    usage();
    std::cout<<"Bundle Info:"<<std::endl;
    typedef std::vector<std::pair<void*,std::string>>::iterator dlIter;
    for (dlIter iter = dlHanle.begin(); iter != dlHanle.end(); iter++) {
        std::cout<<colorize(std::string("[Bundle]"),ColorIO_Color::Red,ColorIO_Style::Highlight)<<iter->second<<std::endl;
        std::cout<<colorize(std::string("ForceType    :"),ColorIO_Color::Cyan,ColorIO_Style::Highlight);
        for (std::vector<assConverterForceType>::iterator i =force_list.begin();
             i != force_list.end(); i++) {
            std::cout<< i -> what<< " ";
        }
        std::cout<<std::endl;
        std::cout<<colorize(std::string("ForceRawType :"),ColorIO_Color::Cyan,ColorIO_Style::Highlight);
        for (std::vector<assConverterForceRawType>::iterator i =force_raw_list.begin();
             i != force_raw_list.end(); i++) {
            std::cout<< i -> what<< " ";
        }
        std::cout<<std::endl;
        std::cout<<colorize(std::string("ForceType    :"),ColorIO_Color::Cyan,ColorIO_Style::Highlight);
        for (std::vector<assConverterFXStrType>::iterator i =fxStr_list.begin();
             i != fxStr_list.end(); i++) {
            std::cout<< i -> what<< " ";
        }
        std::cout<<std::endl;
    }
}
assConverterForceType retriveForceType(std::string _name){
    typedef std::vector<assConverterForceType>::iterator iterType;
    for (iterType iter = force_list.begin(); iter != force_list.end(); iter++){
        if (iter->what == _name) return *iter;
    }
    throw runtime_error(AssCompressorCLI_FUNCNOTEXIST);
}
assConverterForceRawType retriveForceRawType(std::string _name){
    typedef std::vector<assConverterForceRawType>::iterator iterType;
    for (iterType iter = force_raw_list.begin(); iter != force_raw_list.end(); iter++){
        if (iter->what == _name) return *iter;
    }
    throw runtime_error(AssCompressorCLI_FUNCNOTEXIST);
}
assConverterFXStrType retriveFXStrType(std::string _name){
    typedef std::vector<assConverterFXStrType>::iterator iterType;
    for (iterType iter = fxStr_list.begin(); iter != fxStr_list.end(); iter++){
        if (iter->what == _name) return *iter;
    }
    throw runtime_error(AssCompressorCLI_FUNCNOTEXIST);
}

time_t getCorrectRollDate(const size_t _cid, time_t _target){
    char* urlBuffer = (char*)malloc(sizeof(char)*AssCompressorCLI_URL_BUFFER);
    if (urlBuffer == nullptr) throw runtime_error(AssCompressorCLI_NOTENOUGHROOM);
    sprintf(urlBuffer, "http://comment.bilibili.com/rolldate,%lu", _cid);
    std::string rollInfo = netFetcher::quickFetch(urlBuffer);
    free(urlBuffer);
    rapidjson::Document jsonHandler;
    jsonHandler.Parse<rapidjson::kParseDefaultFlags>(rollInfo.c_str());
    if (jsonHandler.HasParseError()) throw runtime_error(AssCompressorCLI_ROLLBACK);
    assert(jsonHandler.IsArray());
    vector<time_t> possibleTime;
    for (rapidjson::Value::ConstValueIterator iter = jsonHandler.Begin();
         iter != jsonHandler.End(); iter++) {
        const rapidjson::Value& infoPair = *iter;
        assert(infoPair["timestamp"].IsString());
        possibleTime.push_back((time_t)atol(infoPair["timestamp"].GetString()));
    }
    jsonHandler.Clear();
    assert(!possibleTime.empty());
    time_t returnT = possibleTime[0];
    long diff = abs((long)returnT - (long)_target);
    for (vector<time_t>::iterator iter = possibleTime.begin(); iter != possibleTime.end(); iter++){
        long currentDiff = abs((long)(*iter) - (long)_target);
        returnT = (currentDiff < diff) ? (*iter) : returnT;
        diff = (currentDiff < diff) ? currentDiff : diff;
    }
    return returnT;
}
inline bool parseBoolString(const char* _cstr){
    // True: true ; TRUE; T; Y; Yes
    // False: flase; FALSE; F; N; No
    // _cstr is not case sensitive.
    assert(_cstr!=nullptr);
    const size_t _cstrLength = strlen(_cstr);
    bool returnBool = false;
    char* bufferCAPTIAL = (char*)malloc(sizeof(char)*(_cstrLength+1));
    for (size_t iter = 0; iter < _cstrLength; iter++) {
        bufferCAPTIAL[iter] = (char)toupper((int)_cstr[iter]);
    }
    returnBool = (strcmp(bufferCAPTIAL, "TRUE") == 0) || (strcmp(bufferCAPTIAL, "T") == 0) ||
                 (strcmp(bufferCAPTIAL, "YES")  == 0) || (strcmp(bufferCAPTIAL, "Y") == 0);
    free(bufferCAPTIAL);
    return returnBool;
}

int main(int argc, char** args){
    try {
        vector<std::string> argsTokens;
        for (int iter = 1; iter < argc; iter++) argsTokens.push_back(std::string(args[iter]));
        bool versionMode = (argc == 1)? true : false;
        bool badArgs = false;
        bool stdinScan = true;
        bool stdOutput = true;
        std::string configureXMl;
        std::string libXML = "library.xml";
        std::string outputPath;
        for (vector<std::string>::iterator iter = argsTokens.begin(); iter!=argsTokens.end(); iter++) {
            if (strcmp(iter->c_str(), "-v") == 0) {versionMode = true; continue;}
            if (strcmp(iter->c_str(), "-l") == 0) {
                if ((++iter) == argsTokens.end()) {badArgs = true; break;}
                libXML = *iter;
                if (!boost::filesystem::exists(libXML.c_str())) {badArgs = true; break;}
                continue;
            }
            if (strcmp(iter->c_str(), "-i") == 0) {
                if ((++iter) == argsTokens.end()) {badArgs = true; break;}
                configureXMl = *iter;
                if (!stdinScan) {badArgs = true; break;}
                stdinScan = false;
                if (!boost::filesystem::exists(configureXMl.c_str())) {badArgs = true; break;}
                continue;
            }
            if (strcmp(iter->c_str(), "-o") == 0) {
                if ((++iter) == argsTokens.end()) {badArgs = true; break;}
                outputPath = *iter;
                if (!stdOutput) {badArgs = true; break;}
                stdOutput = false;
                continue;
            }
            if (strcmp(iter->c_str(), "-stdin") == 0) {stdinScan = true; continue;}
        }
        if (versionMode) {loadAdditionForceFunc(libXML.c_str()); version(); unload(); return 0;}
        if (badArgs) {usage(); return 1;}
        char* buffer = (char*)malloc(sizeof(char)*AssCompressorCLI_MAX_BUFFER);
        memset(buffer, 0, sizeof(char)*AssCompressorCLI_MAX_BUFFER);
        if (stdinScan) {
            size_t ptr = 0;
            char tempChar;
            while ((tempChar = getchar()) != EOF) {
                buffer[ptr] = tempChar; ptr++;
            }
        }else{
            rapidxml::file<> fileHandle(configureXMl.c_str());
            strcpy(buffer, fileHandle.data());
        }
        rapidxml::xml_document<> xmlHandler;
        xmlHandler.parse<0>(buffer);
        rapidxml::xml_node<>* rootNode = xmlHandler.first_node();
        rapidxml::xml_node<>* aidNode = rootNode->first_node("aid");
        rapidxml::xml_node<>* indexNode = rootNode->first_node("index");
        rapidxml::xml_node<>* cidNode = rootNode->first_node("cid");
        rapidxml::xml_node<>* webPageURLNode = rootNode->first_node("webpageURL");
        rapidxml::xml_node<>* commentPageNode = rootNode->first_node("commentPageURL");
        rapidxml::xml_node<>* rollBackNode = rootNode->first_node("rollBack");
        
        std::string bufferedXMLData;
        time_t rollBackTarget = 0L;
        if (rollBackNode != nullptr) rollBackTarget=(time_t)atol(rollBackNode->first_node()->value());
        if (aidNode == nullptr && cidNode == nullptr &&
            webPageURLNode ==nullptr && commentPageNode == nullptr) {
            throw runtime_error(AssCompressorCLI_BADCONFGURE);
        }
        
        assUtil::assConfigure configure;
        
        if (commentPageNode != nullptr) {
            configure.bilibiliTitle = std::string("[Unknown]");
            configure.bilibiliCommentUrl = std::string(commentPageNode->first_node()->value());
            bufferedXMLData = netFetcher::quickFetch(commentPageNode->first_node()->value());
            if (rollBackTarget != 0L) {
                char* buffer = (char*)malloc(sizeof(char)*(bufferedXMLData.length()+1));
                strcpy(buffer, bufferedXMLData.c_str());
                rapidxml::xml_document<> tempDoc;
                tempDoc.parse<0>(buffer);
                rapidxml::xml_node<>* rootNode2 = tempDoc.first_node();
                assert(rootNode2 != nullptr);
                rapidxml::xml_node<>* chatIDNode = rootNode2->first_node("chatid");
                assert(chatIDNode != nullptr);
                size_t cid = (size_t)atol(chatIDNode->first_node()->value());
                tempDoc.clear();
                free(buffer);
                rollBackTarget = getCorrectRollDate(cid, rollBackTarget);
                char* urlBuffer = (char*)malloc(sizeof(char)*AssCompressorCLI_URL_BUFFER);
                sprintf(urlBuffer, "http://comment.bilibili.com/dmroll,%lu,%lu",
                        rollBackTarget, cid);
                bufferedXMLData = netFetcher::quickFetch(urlBuffer);
                configure.bilibiliCommentUrl = std::string(urlBuffer);
                free(urlBuffer);
            }
        }
        if (webPageURLNode != nullptr) {
            std::string webURL(webPageURLNode->first_node()->value());
            bilibiliWebManager webHandle(webURL);
            webHandle.fetchCommentXML();
            configure.bilibiliTitle = webHandle.getTitle();
            configure.bilibiliCommentUrl = webHandle.getCommentURL();
            if (rollBackTarget != 0L) {
                rollBackTarget = getCorrectRollDate(atol(webHandle.getCID().c_str()), rollBackTarget);
                char* urlBuffer = (char*)malloc(sizeof(char)*AssCompressorCLI_URL_BUFFER);
                sprintf(urlBuffer, "http://comment.bilibili.com/dmroll,%lu,%s",
                        rollBackTarget, webHandle.getCID().c_str());
                bufferedXMLData = netFetcher::quickFetch(urlBuffer);
                configure.bilibiliCommentUrl = std::string(urlBuffer);
                free(urlBuffer);
            } else {
                bufferedXMLData = webHandle.getBufferedCommentXML();
            }
        }
        if (cidNode != nullptr) {
            size_t cid = (size_t)atol(cidNode->first_node()->value());
            configure.bilibiliTitle = std::string("[Unknown]");
            if (rollBackTarget != 0L) {
                rollBackTarget = getCorrectRollDate(cid, rollBackTarget);
                char* urlBuffer = (char*)malloc(sizeof(char)*AssCompressorCLI_URL_BUFFER);
                sprintf(urlBuffer, "http://comment.bilibili.com/dmroll,%lu,%lu",
                        rollBackTarget, cid);
                bufferedXMLData = netFetcher::quickFetch(urlBuffer);
                configure.bilibiliCommentUrl = std::string(urlBuffer);
                free(urlBuffer);
            } else {
                char* urlBuffer = (char*)malloc(sizeof(char)*AssCompressorCLI_URL_BUFFER);
                sprintf(urlBuffer, "http://comment.bilibili.tv/%lu.xml", cid);
                configure.bilibiliCommentUrl = std::string(urlBuffer);
                bufferedXMLData = netFetcher::quickFetch(urlBuffer);
                free(urlBuffer);
            }
        }
        if (aidNode != nullptr) {
            size_t aid = (size_t)atol(aidNode->first_node()->value());
            size_t index = 1L;
            if (indexNode != nullptr) index = (size_t)atol(indexNode->first_node()->value());
            bilibiliWebManager webHandle((bilibiliWebManager::bilibiliURL(aid, index)));
            webHandle.fetchCommentXML();
            configure.bilibiliTitle = webHandle.getTitle();
            configure.bilibiliCommentUrl = webHandle.getCommentURL();
            if (rollBackTarget != 0L) {
                rollBackTarget = getCorrectRollDate(atol(webHandle.getCID().c_str()), rollBackTarget);
                char* urlBuffer = (char*)malloc(sizeof(char)*AssCompressorCLI_URL_BUFFER);
                sprintf(urlBuffer, "http://comment.bilibili.com/dmroll,%lu,%s",
                        rollBackTarget, webHandle.getCID().c_str());
                bufferedXMLData = netFetcher::quickFetch(urlBuffer);
                configure.bilibiliCommentUrl = std::string(urlBuffer);
                free(urlBuffer);
            } else {
                bufferedXMLData = webHandle.getBufferedCommentXML();
            }
        }
        
        rapidxml::xml_node<>* canvasXNode = rootNode->first_node("canvasX");
        rapidxml::xml_node<>* canvasYNode = rootNode->first_node("canvasY");
        rapidxml::xml_node<>* durationNode = rootNode->first_node("duration");
        rapidxml::xml_node<>* playerXNode = rootNode->first_node("playerX");
        rapidxml::xml_node<>* playerYNode = rootNode->first_node("playerY");
        rapidxml::xml_node<>* vidRatioXNode = rootNode->first_node("vidRatioX");
        rapidxml::xml_node<>* vidRatioYNode = rootNode->first_node("vidRatioY");
        rapidxml::xml_node<>* defaultFontNameNode = rootNode->first_node("defaultFontName");
        rapidxml::xml_node<>* defaultFontSizeNode = rootNode->first_node("defaultFontSize");
        rapidxml::xml_node<>* layerRollTypeNode = rootNode->first_node("layerRollType");
        rapidxml::xml_node<>* layerFixTypeNode = rootNode->first_node("layerFixType");
        rapidxml::xml_node<>* layerACCTypeNode = rootNode->first_node("layerACCType");
        rapidxml::xml_node<>* setSubtitleTopNode = rootNode->first_node("setSubtitleTop");
        rapidxml::xml_node<>* multiLineCollisionNode = rootNode->first_node("multiLineCollision");
        rapidxml::xml_node<>* multiLineCommentNode = rootNode->first_node("multiLineComment");
        rapidxml::xml_node<>* borderSizeNode = rootNode->first_node("borderSize");
        rapidxml::xml_node<>* fadeScalarNode = rootNode->first_node("fadeScalar");
        rapidxml::xml_node<>* forceTypeNode = rootNode->first_node("forceType");
        rapidxml::xml_node<>* forceRawTypeNode = rootNode->first_node("forceRawType");
        rapidxml::xml_node<>* forceFXStrNode = rootNode->first_node("forceFXStr");
        
        if (canvasXNode != nullptr)
            configure.canvasX = (size_t)atol(canvasXNode->first_node()->value());
        if (canvasYNode != nullptr)
            configure.canvasY = (size_t)atol(canvasYNode->first_node()->value());
        if (durationNode != nullptr)
            configure.duration = (double)atof(durationNode->first_node()->value());
        if (playerXNode != nullptr)
            configure.playerX = (size_t)atol(playerXNode->first_node()->value());
        if (playerYNode != nullptr)
            configure.playerY = (size_t)atol(playerYNode->first_node()->value());
        if (vidRatioXNode != nullptr)
            configure.vidRatioX = (size_t)atol(vidRatioXNode->first_node()->value());
        if (vidRatioYNode != nullptr)
            configure.vidRatioY = (size_t)atol(vidRatioYNode->first_node()->value());
        if (defaultFontNameNode != nullptr)
            configure.defaultFontName = std::string(defaultFontNameNode->first_node()->value());
        if (defaultFontSizeNode != nullptr)
            configure.defaultFontSize = (size_t)atol(defaultFontSizeNode->first_node()->value());
        if (layerRollTypeNode != nullptr)
            configure.layerRollType = (int)atoi(layerRollTypeNode->first_node()->value());
        if (layerFixTypeNode != nullptr)
            configure.layerFixType = (int)atoi(layerFixTypeNode->first_node()->value());
        if (layerACCTypeNode != nullptr)
            configure.layerACCType = (int)atoi(layerACCTypeNode->first_node()->value());
        if (setSubtitleTopNode != nullptr)
            configure.setSubtitileOnTop = parseBoolString(setSubtitleTopNode->first_node()->value());
        if (multiLineCollisionNode != nullptr)
            configure.multiLineCollision = parseBoolString(multiLineCollisionNode->
                                                           first_node()->value());
        if (multiLineCommentNode != nullptr)
            configure.multiLineComment = parseBoolString(multiLineCommentNode->
                                                         first_node()->value());
        if (borderSizeNode != nullptr)
            configure.borderSize = (double)atof(borderSizeNode->first_node()->value());
        if (fadeScalarNode != nullptr) {
            configure.fadeConfigArray = (double*)malloc(sizeof(double)*9);
            size_t ptr = 0;
            const char* jsonStr = fadeScalarNode->first_node()->value();
            rapidjson::Document jsonHandler;
            jsonHandler.Parse<rapidjson::kParseDefaultFlags>(jsonStr);
            if (jsonHandler.HasParseError()) throw runtime_error(AssCompressorCLI_BADCONFGURE);
            assert(jsonHandler.IsArray());
            for (rapidjson::Value::ConstValueIterator iter = jsonHandler.Begin();
                 iter != jsonHandler.End(); iter++) {
                const rapidjson::Value& val = *iter;
                assert(val.IsNumber());
                configure.fadeConfigArray[++ptr] = val.GetDouble();
            }
        }
    
        loadAdditionForceFunc(libXML.c_str());
        assConverter converter(configure);
        if (forceTypeNode != nullptr) {
            const char* jsonStr = forceTypeNode->first_node()->value();
            rapidjson::Document jsonHandler;
            jsonHandler.Parse<0>(jsonStr);
            if (jsonHandler.HasParseError()) throw runtime_error(AssCompressorCLI_BADCONFGURE);
            assert(jsonHandler.IsArray());
            typedef rapidjson::Value::ConstValueIterator iterType;
            for (iterType iter = jsonHandler.Begin(); iter!=jsonHandler.End(); iter++){
                assert(iter->IsString());
                converter.queue_force(retriveForceType(std::string(iter->GetString())));
            }
        }
        if (forceRawTypeNode != nullptr) {
            const char* jsonStr = forceRawTypeNode->first_node()->value();
            rapidjson::Document jsonHandler;
            jsonHandler.Parse<0>(jsonStr);
            if (jsonHandler.HasParseError()) throw runtime_error(AssCompressorCLI_BADCONFGURE);
            assert(jsonHandler.IsArray());
            typedef rapidjson::Value::ConstValueIterator iterType;
            for (iterType iter = jsonHandler.Begin(); iter!=jsonHandler.End(); iter++){
                assert(iter->IsString());
                converter.queue_force_raw(retriveForceRawType(std::string(iter->GetString())));
            }
        }
        if (forceFXStrNode != nullptr) {
            const char* jsonStr = forceFXStrNode->first_node()->value();
            rapidjson::Document jsonHandler;
            jsonHandler.Parse<0>(jsonStr);
            if (jsonHandler.HasParseError()) throw runtime_error(AssCompressorCLI_BADCONFGURE);
            assert(jsonHandler.IsArray());
            typedef rapidjson::Value::ConstValueIterator iterType;
            for (iterType iter = jsonHandler.Begin(); iter!=jsonHandler.End(); iter++){
                assert(iter->IsString());
                converter.queue_FXStr(retriveFXStrType(std::string(iter->GetString())));
            }
        }
    
        converter.analysis(bufferedXMLData);
        std::string bufferedOutput = converter.assFile();
        if (stdOutput) {
            std::cout<<bufferedOutput<<std::endl;
        }else{
            ofstream outputStream(outputPath);
            outputStream<<bufferedOutput;
            outputStream.close();
        }
        
        if (stdinScan) free(buffer);
        if (configure.fadeConfigArray != assUtil::defaultFadeScale) free(configure.fadeConfigArray);
        unload();
        return 0;
    } catch (runtime_error& _err) {
        std::cerr<<colorize("[Error] ", ColorIO_Color::Red, ColorIO_Style::Highlight)<<_err.what()<<std::endl;
        return 1;
    } catch (...){
        std::cerr<<colorize("[Error] Unexpected Error", ColorIO_Color::Red, ColorIO_Style::Highlight)<<std::endl;
        return 1;
    }
}