#ifndef AssCompressor_commentPlaceStream_H
#define AssCompressor_commentPlaceStream_H

#include <exception>
#include <limits>
/*
        +-----------------------------------------------------------+--
        |                              +---------------+            | ^
        |                              | Collision box |            | |
        |                              +---------------+            | |
        |                                                           | |
        |                         Screen                            | Canvas Y
        |                                                           | |
        |                                                           | |
        |                                                           | |
        |                                                           | |
        +-----------------------------------------------------------+--
        |<------------------------Canvas X------------------------->|
 
 Class CommentPlaceStream
    A simutation of the position allocator of the ROLL type comment, note that 
 the return value is depent on the order of the input sequence, and each 
 collision box may have different size.
 
 Usage: 
commentPlaceStream(size_t _x,size_t _y,double (*velocityCal)(size_t),void* _cfd)
 |  _x -> Canvas X
 |  _y -> Canvas Y
 |  velocityCal ->  function takes the length of a collision box, and return its
                    velocity, in double. Example:
 
                    template<size_t _x> double example(size_t _l
                                                       void* _duration){
                        const double _d = *((double*)_duration);
                        if (_l <= 12*3) {
                            return (_x+12*3)/((_x / 512.0)*_d);
                        }else if(_l <= 15*3){
                            return (_x+15*3)/((_x / 512.0)*_d);
                        }else{
                            return (_x+_l)/((_x / 512.0)*_d);
                    }
 
                    This stream mimic the velocity used by Bilibili SWF Player:
 
                    double duration = 5.0;
                    double (*func)(size_t, void*) = example<_x>;
                    commentPlaceStream stream(_x, _y, func, &duration);
 
                    The duration for each comment is 5.0s on such function.
 |  _cfd -> A void pointer to the data carry forward, this data will be pass to
            the velocity calcuation function everytime it called.
 
 size_t fetch(double _initTime, size_t _length, size_t _height)
 |  _initTime -> time when the collision box left sided entered
 |  _length   -> the length of the current collision box
 |  _height   -> the height of the current collision box
 
 Build a StaticCommentPlaceStream out of CommentPlaceStream
 Initalize:
    commentPlaceStream(0, size_t _y, valCalfunc)
 |  -y -> canvas Y
 |  valCalfunc ->   double valCalfunc(size_t _length, void* _d){
                        return _length / *(((double*)_d));
                    }
                    
                    This function will allow each static comment to stay on
                    screen for 5.0 seconds.
 */

class commentPlaceStream{
    size_t canvasX;
    size_t canvasY;
    double (*calVelocity)(size_t, void*);
    double* pixelMap;
    void* dataCF;
    double calFontReachTime(double _initTime, size_t _length){
        assert(calVelocity!=nullptr);
        return _initTime + (this->canvasX / calVelocity(_length, dataCF));
    }
    double calBackReachTime(double _initTime, size_t _length){
        return _initTime+((this->canvasX+_length)/calVelocity(_length,dataCF));
    }
    double calChunkTime(size_t _s, size_t _e){
        double maxVal = std::numeric_limits<double>::min();
        for (size_t iter = _s; iter < _e; iter++) {
            maxVal = (pixelMap[iter] > maxVal)? pixelMap[iter] : maxVal;
        }
        return maxVal;
    }
    void markDown(size_t _s, size_t _e, double _initTime, size_t _length){
        for (size_t iter = _s; iter < _e; iter++) {
            pixelMap[iter] = calBackReachTime(_initTime, _length);
        }
    }
    
public:
    size_t fetch(double _initTime, size_t _length, size_t _height){
        double frontTime = calFontReachTime(_initTime, _length);
        double minTime = std::numeric_limits<double>::max();
        size_t chunkPos = 0;
        for (size_t iter = 0; iter < this->canvasY - _height; iter++) {
            double chunkTime = calChunkTime(iter, iter+_height);
            if (frontTime >= chunkTime){
                markDown(iter, iter+_height, _initTime, _length);
                return iter;
            }
            if (chunkTime < minTime) {
                minTime = chunkTime;
                chunkPos = iter;
            }
        }
        markDown(chunkPos, chunkPos+_height, _initTime, _length);
        return chunkPos;
    }
    commentPlaceStream(size_t _x, size_t _y,
                       double(*calf)(size_t, void*),
                       void* _dataCF){
        this->canvasX = _x;
        this->canvasY = _y;
        this->calVelocity = calf;
        this->dataCF = _dataCF;
        
        this->pixelMap = (double*)malloc(sizeof(double) * _y);
        if (this->pixelMap == nullptr){
            throw std::runtime_error(COMMENTPLACESTREAM_NOT_ENOUGH_ROOM);
        }
        
        for (size_t iter = 0; iter < _y; iter++) {
            this->pixelMap[iter] = 0.0;
        }
    }
    void reset(){
        for (size_t iter = 0; iter < this->canvasY; iter++) {
            this->pixelMap[iter] = 0;
        }
    }
    ~commentPlaceStream(){free(this->pixelMap);}
};
#endif