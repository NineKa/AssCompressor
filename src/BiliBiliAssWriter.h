#ifndef __Class__BiliBiliAssWriter__
#define __Class__BiliBiliAssWriter__

#include "BiliBiliCommentArray.h"
#include <vector>
#include <cmath>
#include <string>

class BiliBiliAssWriter{
public:
    BiliBiliAssWriter(){};
    BiliBiliAssWriter(BiliBiliCommentArray& _ref) {this->writeFrom(_ref);}
    void writeFrom(BiliBiliCommentArray& _ref){this->reference = _ref;}
    
    static void reduceBasic(BiliBiliCommentArray&);
    
protected:
    BiliBiliCommentArray& reference = BiliBiliAssWriter::empty;
private:
    static BiliBiliCommentArray empty;
    static void copyBiliBiliCommentContainer(BiliBiliCommentContainer*, BiliBiliCommentContainer*);
    static void reduceErrorArthematic(std::vector<BiliBiliCommentContainer>&);
    static unsigned long totalDistanceMoved(const std::vector<TracePos>&);
    static unsigned long distanceBetween(const TracePos&, const TracePos&);
};

void BiliBiliAssWriter::reduceBasic(BiliBiliCommentArray& _arr){
    std::vector<BiliBiliCommentContainer>& _arrRef = _arr.getFetchedData();
    for (size_t i=0; i<_arrRef.size(); i++){
        if (_arrRef[i].getType() != BiliBiliCommentContainer::BiliBiliCommentType::type_positionedComment){continue;}
        BiliBiliCommentContainer_PositionedDetail& detailInfo = _arrRef[i].getPositionedDetial();
        if (detailInfo.movingPauseMS!=0) {          //Before the Comment moves, it waits for several microseconds
            double movingPause = static_cast<double>(detailInfo.movingPauseMS)/1000;
            BiliBiliCommentContainer current;
            BiliBiliCommentContainer furtherReduce;
            BiliBiliAssWriter::copyBiliBiliCommentContainer(&current, &_arrRef[i]);
            BiliBiliAssWriter::copyBiliBiliCommentContainer(&furtherReduce, &_arrRef[i]);
            double fadeStart = _arrRef[i].getPositionedDetial().fadeinColor;
            double fadeEnd = _arrRef[i].getPositionedDetial().fadeoutColor;
            fadeEnd = ((fadeEnd-fadeStart)/(detailInfo.duration))*movingPause+fadeStart;
            BiliBiliCommentContainer_PositionedDetail newDetailCurrent = _arrRef[i].getPositionedDetial();
            BiliBiliCommentContainer_PositionedDetail furtherReduceDetail = _arrRef[i].getPositionedDetial();
            newDetailCurrent.duration = movingPause;            //reset every thing
            newDetailCurrent.fadeinColor = fadeStart;
            newDetailCurrent.fadeoutColor = fadeEnd;
            newDetailCurrent.movePosX = detailInfo.initPosX;
            newDetailCurrent.movePosY = detailInfo.initPosY;
            newDetailCurrent.movingDurationMS = movingPause;
            newDetailCurrent.movingPauseMS = 0L;
            newDetailCurrent.traceLine.clear();
            
            furtherReduceDetail.duration = detailInfo.duration-movingPause;
            furtherReduceDetail.fadeinColor = fadeEnd;
            furtherReduceDetail.movingPauseMS = 0L;
            furtherReduce.setTime(_arrRef[i].getTime()+movingPause);
            current.setPostitionedDetial(newDetailCurrent);
            furtherReduce.setPostitionedDetial(furtherReduceDetail);
            BiliBiliAssWriter::copyBiliBiliCommentContainer(&_arrRef[i], &current);
            //_arrRef[i] = current;
            _arrRef.push_back(furtherReduce);
            continue;
        }
        if (!detailInfo.traceLine.empty()){        // Trace Along the line
            std::vector<BiliBiliCommentContainer> appendWaitList;
            unsigned long totalMoveTime = detailInfo.movingDurationMS;
            unsigned long totalMoveDistance = BiliBiliAssWriter::totalDistanceMoved(detailInfo.traceLine);
            double finalWait = detailInfo.duration - static_cast<double>(totalMoveTime)/1000;
            double averageSpeed = static_cast<double>(totalMoveDistance)/totalMoveTime;
            double fadeStart = detailInfo.fadeinColor;
            double fadeEnd  = detailInfo.fadeoutColor;
            double averageFade = (fadeEnd-fadeStart) / totalMoveDistance;
            for (size_t i = 0; i<detailInfo.traceLine.size()-1; i++) {
                unsigned long currentDisatance = BiliBiliAssWriter::distanceBetween(detailInfo.traceLine[i], detailInfo.traceLine[i+1]);
                double currentFade = averageFade * currentDisatance;
                double currentTime = currentDisatance / averageSpeed;
                BiliBiliCommentContainer commentAppend = _arrRef[i];
                BiliBiliCommentContainer_PositionedDetail modifier = commentAppend.getPositionedDetial();
                modifier.initPosX = detailInfo.traceLine[i].posX;
                modifier.initPosY = detailInfo.traceLine[i].posY;
                modifier.movePosX = detailInfo.traceLine[i+1].posX;
                modifier.movePosY = detailInfo.traceLine[i+1].posY;
                modifier.fadeinColor = fadeStart;
                modifier.fadeoutColor = fadeStart + currentFade;
                fadeStart = modifier.fadeoutColor;
                modifier.duration = currentTime;
                modifier.movingDurationMS = static_cast<unsigned long>(currentTime*1000);
                modifier.movingPauseMS = 0L;
                modifier.traceLine.clear();
                commentAppend.setPostitionedDetial(modifier);
                _arrRef.push_back(commentAppend);
            }
            BiliBiliCommentContainer_PositionedDetail finalModify = _arrRef[_arrRef.size()-1].getPositionedDetial();
            finalModify.duration = finalModify.duration + finalWait;
            _arrRef[_arrRef.size()-1].setPostitionedDetial(finalModify);
            _arrRef[i].setType(BiliBiliCommentContainer::BiliBiliCommentType::type_unknown);
            continue;
        }
        if (detailInfo.duration > (static_cast<double>(detailInfo.movingDurationMS)/1000) && detailInfo.movingDurationMS != 0L){    // There is still time left after moving
            double fadeStart = detailInfo.fadeinColor;
            double fadeEnd = detailInfo.fadeoutColor;
            double averageFade = (fadeEnd - fadeStart) / detailInfo.duration;
            BiliBiliCommentContainer commentAppend = _arrRef[i];
            BiliBiliCommentContainer_PositionedDetail originDetail = _arrRef[i].getPositionedDetial();
            double movingDuration = static_cast<double>(originDetail.movingDurationMS)/1000.0;
            double originalDuration = originDetail.duration;
            originDetail.duration = movingDuration;
            originDetail.fadeinColor = fadeStart;
            originDetail.fadeoutColor = fadeStart + originDetail.duration * averageFade;
            _arrRef[i].setPostitionedDetial(originDetail);
            BiliBiliCommentContainer_PositionedDetail newDetail = commentAppend.getPositionedDetial();
            newDetail.initPosX = originDetail.movePosX;
            newDetail.movePosY = originDetail.movePosY;
            newDetail.duration = originalDuration - movingDuration;
            newDetail.movingDurationMS = static_cast<unsigned long>(newDetail.duration*1000);
            newDetail.fadeinColor = originDetail.fadeoutColor;
            newDetail.fadeoutColor = fadeEnd;
            commentAppend.setPostitionedDetial(newDetail);
            commentAppend.setTime(commentAppend.getTime()+_arrRef[i].getPositionedDetial().duration);
            if (commentAppend.getPositionedDetial().duration!=0){_arrRef.push_back(commentAppend);}
            continue;
        }
        if (detailInfo.duration < (static_cast<double>(detailInfo.movingDurationMS)/1000)){     // Moving time is larger than duration, adjust target position
            int distanceMoveX = detailInfo.initPosX - detailInfo.movePosX;
            int distanceMoveY = detailInfo.initPosY - detailInfo.movePosY;
            distanceMoveX = distanceMoveX * (detailInfo.duration / (static_cast<double>(detailInfo.movingDurationMS)/1000));
            distanceMoveY = distanceMoveY * (detailInfo.duration / (static_cast<double>(detailInfo.movingDurationMS)/1000));
            BiliBiliCommentContainer_PositionedDetail adjustDetail = detailInfo;
            adjustDetail.movePosX = adjustDetail.initPosX - distanceMoveX;
            adjustDetail.movePosY = adjustDetail.initPosY - distanceMoveY;
            _arrRef[i].setPostitionedDetial(adjustDetail);
        }
    }
    BiliBiliAssWriter::reduceErrorArthematic(_arrRef);      //elimate error due to float point arthematics
    _arr.sort(); // During the reduce process the order his been mixed
}

unsigned long BiliBiliAssWriter::totalDistanceMoved(const std::vector<TracePos>& _traceArr){
    #define BilibiliAssWrite_Power2(x) (x*x)
    unsigned long returnValue = 0L;
    for (size_t i=0; i<_traceArr.size()-1; i++) {
        unsigned long currentDistance = 0;
        currentDistance = static_cast<unsigned long>(
            sqrt(BilibiliAssWrite_Power2(_traceArr[i].posX - _traceArr[i+1].posX)+
                 BilibiliAssWrite_Power2(_traceArr[i].posY - _traceArr[i+1].posY)
                 ));
        returnValue+=currentDistance;
    }
    return returnValue;
}

unsigned long BiliBiliAssWriter::distanceBetween(const TracePos& x, const TracePos& y){
    unsigned long returnValue = 0;
    returnValue = static_cast<unsigned long>(
        sqrt(BilibiliAssWrite_Power2(x.posX - y.posX)+
             BilibiliAssWrite_Power2(x.posY - y.posY)));
    return returnValue;
}

void BiliBiliAssWriter::copyBiliBiliCommentContainer(BiliBiliCommentContainer* x, BiliBiliCommentContainer* y){
    x->setTime(y->getTime());
    x->setType(y->getType());
    x->setFontSize(y->getFontSize());
    x->setFontColor(y->getFontColor());
    x->setTimestamp(y->getTimestamp());
    x->setPool(y->getPool());
    std::string userID = y->getUserID();
    x->setUserID(userID);
    std::string commentID = y->getCommentID();
    x->setCommentID(commentID);
    std::string content = y->getContent();
    x->setContent(content);
    x->setPostitionedDetial(y->getPositionedDetial());
}

void BiliBiliAssWriter::reduceErrorArthematic(std::vector<BiliBiliCommentContainer>& _error){
    bool noMore = false;
    while (!noMore) {
        noMore = true;
        for (size_t i=0; i<_error.size(); i++) {
            if (_error[i].getType() == BiliBiliCommentContainer::BiliBiliCommentType::type_positionedComment) {
                if (_error[i].getPositionedDetial().duration <= 0.001){
                    std::swap(_error[i],_error[_error.size()-1]);
                    _error.pop_back();
                    noMore = false;
                }
            }
        }
    }
}
#endif
