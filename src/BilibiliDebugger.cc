#include "BilibiliPageFetcher.h"
#include "BilibiliFetcher.h"
#include "MultiThreadDownloader.h"
#include "BiliBiliCommentArray.h"
#include "BiliBiliAssWriter.h"

#include <iostream>
#include <string>

using namespace std;

string ASS_TIME(double seconds) {
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

int main(int args, char** argv){
    BilibiliPageFetcher test1("http://www.bilibili.com/video/av741641/index_1.html");
    test1.performComment();
    BiliBiliCommentArray test2(test1.getBufferedCommentXML());
    BiliBiliAssWriter::reduceBasic(test2);
    
    test2.sort();
    for (BiliBiliCommentContainer& i : test2.getFetchedData()){
        if (i.getType()!=BiliBiliCommentContainer::BiliBiliCommentType::type_positionedComment){continue;}
        printf("Dialogue: 0,%s,%s,BufferFront,,0000,0000,0000,,{\\an7\\fs%lu\\move(%d,%d,%d,%d)}{\\b1}%s{\\b0}\n",
               ASS_TIME(i.getTime()).c_str(),
               ASS_TIME(i.getTime()+i.getPositionedDetial().duration).c_str(),
               i.getFontSize(),
               i.getPositionedDetial().initPosX,
               i.getPositionedDetial().initPosY,
               i.getPositionedDetial().movePosX,
               i.getPositionedDetial().movePosY,
               i.getContent().c_str()
               );
    }
}