#ifndef __Class__BiliBiliCommentArray__
#define __Class__BiliBiliCommentArray__

#include "BiliBiliCommentContainer.h"
#include "rapidxml/rapidxml.hpp"
#include <vector>
#include <cstring>
#include <algorithm>

class BiliBiliCommentArray{
public:
    BiliBiliCommentArray(){};
    BiliBiliCommentArray(std::string& _xmldata){this->analysis(_xmldata);}
    void analysis(std::string&);
    std::vector<BiliBiliCommentContainer>& getFetchedData(){return this->commentArray;}
    size_t getNumComment(){return this->commentArray.size();}
    void perform(void(*)(std::vector<BiliBiliCommentContainer>&,void*),void*);
    void perform(void(*_func)(std::vector<BiliBiliCommentContainer>&,void*)){this->perform(_func, nullptr);}
    void sort();
    static void bilibiliCommentModify_RemoveByUserID(std::vector<BiliBiliCommentContainer>&, void*);
    static void bilibiliCommentModify_RemoveByCommentID(std::vector<BiliBiliCommentContainer>&, void*);
    static void bilibiliCommentModify_RemoveByColor(std::vector<BiliBiliCommentContainer>&, void*);
    static void bilibiliCommentModify_RemoveByType(std::vector<BiliBiliCommentContainer>&, void*);
    static void bilibiliCommentModify_RemoveByPool(std::vector<BiliBiliCommentContainer>&, void*);
    static void bilibiliCommentModify_RemoveVisitorComment(std::vector<BiliBiliCommentContainer>&, void*);
protected:
    std::vector<BiliBiliCommentContainer> commentArray;
private:
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
    static void quickSort(std::vector<BiliBiliCommentContainer>&, size_t, size_t);
};
void BiliBiliCommentArray::analysis(std::string& _xmlData){
    char* _xmlDataCStr = new char[_xmlData.length()+1];
    strcpy(_xmlDataCStr, _xmlData.c_str());
    rapidxml::xml_document<> xmlDataHandler;
    xmlDataHandler.parse<0>(_xmlDataCStr);
    rapidxml::xml_node<>* rootNode = xmlDataHandler.first_node("i");
    rapidxml::xml_node<>* dataNode = rootNode->first_node("d");
    while (dataNode!=nullptr){
        if (strcmp(dataNode->name(), "d")!=0) {dataNode = dataNode->next_sibling();}
        std::string attributeValue = dataNode->first_attribute()->value();
        std::string contentValue = dataNode->value();
        dataNode = dataNode->next_sibling();
        BiliBiliCommentContainer commentToAdd;
        this->commentArray.push_back(commentToAdd);
        try{
            commentArray[commentArray.size()-1].analysis(attributeValue, contentValue);
        }catch(std::runtime_error& err){
            if (strcmp(err.what(), Error_BiliBiliCommentContainer_DecodeACC)==0){
                commentArray[commentArray.size()-1].setType(BiliBiliCommentContainer::BiliBiliCommentType::type_unknown);
            }
        }
    }
    delete[] _xmlDataCStr;
    this->commentArray.shrink_to_fit();
}
void BiliBiliCommentArray::sort(){BiliBiliCommentArray::quickSort(this->commentArray, 0, commentArray.size()-1);}
void BiliBiliCommentArray::quickSort(std::vector<BiliBiliCommentContainer>& _arr, size_t beg, size_t end){
    size_t i = beg, j = end;
    BiliBiliCommentContainer tmp;
    double pivot = _arr[(beg+end)/2].getTime();
    while (i <= j) {
        while (_arr[i].getTime() < pivot) i++;
        while (_arr[j].getTime() > pivot) j--;
        if (i <= j) {
            tmp = _arr[i];
            _arr[i] = _arr[j];
            _arr[j] = tmp;
            i++;
            j--;
        }
    };
    if (beg < j) BiliBiliCommentArray::quickSort(_arr, beg, j);
    if (i < end) BiliBiliCommentArray::quickSort(_arr, i, end);
}
void BiliBiliCommentArray::perform(void(*_func)(std::vector<BiliBiliCommentContainer>&,void*),void* _arg){_func(this->commentArray, _arg);}
void BiliBiliCommentArray::bilibiliCommentModify_RemoveByType(std::vector<BiliBiliCommentContainer>& _arr, void* _type){
    BiliBiliCommentContainer::BiliBiliCommentType _typeCAST = *(static_cast<BiliBiliCommentContainer::BiliBiliCommentType*>(_type));
    for (size_t i = 0; i<_arr.size(); i++){
        if (_arr[i].getType() == _typeCAST){
            std::swap(_arr[i], _arr[_arr.size()-1]);
            _arr.pop_back();
        }
    }
}
void BiliBiliCommentArray::bilibiliCommentModify_RemoveByPool(std::vector<BiliBiliCommentContainer>& _arr, void* _pool){
    BiliBiliCommentContainer::BilibiliCommentPool _poolCAST = *(static_cast<BiliBiliCommentContainer::BilibiliCommentPool*>(_pool));
    for (size_t i = 0; i<_arr.size(); i++){
        if (_arr[i].getPool() == _poolCAST){
            std::swap(_arr[i], _arr[_arr.size()-1]);
            _arr.pop_back();
        }
    }
}
void BiliBiliCommentArray::bilibiliCommentModify_RemoveByColor(std::vector<BiliBiliCommentContainer>& _arr, void* _color){
    unsigned long _colorCAST = *(static_cast<unsigned long*>(_color));
    for (size_t i = 0; i<_arr.size(); i++){
        if (_arr[i].getFontColor() == _colorCAST){
            std::swap(_arr[i], _arr[_arr.size()-1]);
            _arr.pop_back();
        }
    }
}
void BiliBiliCommentArray::bilibiliCommentModify_RemoveByUserID(std::vector<BiliBiliCommentContainer>& _arr, void* _userid){
    std::string* _useridPTR = static_cast<std::string*>(_userid);
    for (size_t i = 0; i<_arr.size(); i++){
        if (_arr[i].getUserID() == *_useridPTR){
            std::swap(_arr[i], _arr[_arr.size()-1]);
            _arr.pop_back();
        }
    }
}
void BiliBiliCommentArray::bilibiliCommentModify_RemoveByCommentID(std::vector<BiliBiliCommentContainer>& _arr, void* _commentid){
    std::string* _commentidPTR = static_cast<std::string*>(_commentid);
    for (size_t i = 0; i<_arr.size(); i++) {
        if (_arr[i].getCommentID() == *_commentidPTR){
            std::swap(_arr[i], _arr[_arr.size()-1]);
            _arr.pop_back();
        }
    }
}
void BiliBiliCommentArray::bilibiliCommentModify_RemoveVisitorComment(std::vector<BiliBiliCommentContainer>& _arr, void*){
    for (size_t i = 0; i<_arr.size(); i++){
        if (_arr[i].isVisitorComment()){
            std::swap(_arr[i], _arr[_arr.size()-1]);
            _arr.pop_back();
        }
    }
}
#endif
