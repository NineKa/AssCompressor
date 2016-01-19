#ifndef AssCompressor_assUtil_H
#define AssCompressor_assUtil_H

#include "configure.h"
#include <string>
#include <cstdlib>
#include <utility>
#include <cmath>

namespace assUtil {
    const size_t bilibiliPlayerLegacyX = 524;
    const size_t bilibiliPlayerLegacyY = 386;
    double defaultFadeScale[9] = {1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0};
    struct assConfigure{
        size_t canvasX = 524;
        size_t canvasY = 295;
        double duration = 7.0;
        size_t playerX = 524;
        size_t playerY = 396;
        size_t vidRatioX = 16;
        size_t vidRatioY = 9;
        
        std::string bilibiliTitle = std::string("");
        std::string bilibiliCommentUrl = std::string("");
        std::string defaultFontName =std::string(BilibiliFont_Default_FontName);
        size_t defaultFontSize = 18;
        
        int layerRollType = -3;
        int layerFixType  = -2;
        int layerACCType  = -1;
        bool setSubtitileOnTop = true;
        
        bool multiLineCollision = true;
        bool multiLineComment   = true;
        
        double borderSize = 0.5;
        /*
         +---+---------------------------------------------------------+
         | 0 | Default                                                 |
         | 1 | TopRollComment                                          |
         | 2 | BottomRollComment                                       |
         | 4 | BottomFixComment                                        |
         | 5 | TopFixComment                                           |
         | 6 | ReverseTopRollComment                                   |
         | 7 | FXComment                                               |
         | 8 | CodeComment           (NOT SUPPORT FOR CURRENT VERSION) |
         +---+---------------------------------------------------------+
                                                                      */
        double* fadeConfigArray = defaultFadeScale;
    };
    std::string assTime(double seconds) {
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
        return std::string(time_stamp);
    }
    std::string assColor(int _color){
        char* tempBuffer = new char[7];
        sprintf(tempBuffer, "%06x", _color);
        char* newBuffer = new char[7];
        newBuffer[0] = tempBuffer[4];
        newBuffer[1] = tempBuffer[5];
        newBuffer[2] = tempBuffer[2];
        newBuffer[3] = tempBuffer[3];
        newBuffer[4] = tempBuffer[0];
        newBuffer[5] = tempBuffer[1];
        std::string returnStr(newBuffer);
        delete tempBuffer;
        delete newBuffer;
        return returnStr;
    }
    std::string assFade(const double _fade){
        unsigned int returnValue = 255 - static_cast<unsigned int>(255*_fade);
        char* tempBuffer = new char[4];
        sprintf(tempBuffer, "%d", returnValue);
        std::string returnStr(tempBuffer);
        delete [] tempBuffer;
        return returnStr;
    }
    size_t acturalLength(const char* _str){
        size_t length = 0;
        while (*_str) length += (*_str++ & 0xc0) != 0x80;
        return length;
    }
    template <size_t _x> double defaultRollVelocityFunc(size_t _length,
                                                        void* _duration){
        double duration = *((int*)_duration);
        if (_length <= 12*3) {
            return (_x+12*3)/((_x / 524.0)*duration);
        }else if(_length <= 15*3){
            return (_x+15*3)/((_x / 524.0)*duration);
        }else{
            return (_x+_length)/((_x / 524.0)*duration);
        }
    }
    double defaultStaticVelocityFunc(size_t _length, void* _duration){
        return _length/ (*((double*)_duration));
    }
    
    std::pair<long, long>ACCPosAdjust(size_t _x,         size_t _y,
                                      size_t _vidRatioX, size_t _vidRatioY,
                                      size_t _playerX,   size_t _playerY,
                                      size_t _canvasX,   size_t _canvasY){
        double videoRatio = (double)_vidRatioX / (double)_vidRatioY;
        double playerRatio= (double)_playerX / _playerY;
        if (playerRatio < videoRatio){
            // Back Margin is added to top and bottom
            long blackMargin =
           (long)floor(_playerY-((_playerX/((double)_vidRatioX))*_vidRatioY));
            long returnX = ((_x / ((double)_playerX)) * _canvasX);
            long returnY = ((((long)_y - blackMargin / 2) /
                             ((double)(_playerY-blackMargin)))*_canvasY);
            return std::pair<long,long>(returnX, returnY);
        }else{
            // Back Margin is added to left and right
            long blackMargin =
           (long)floor(_playerX-((_playerY/((double)_vidRatioY))*_vidRatioX));
            long returnX = ((((long)_x - blackMargin / 2) /
                             ((double)(_playerX-blackMargin)))*_canvasX);
            long returnY = ((_y / ((double)_playerY)) * _canvasY);
            return std::pair<long,long>(returnX, returnY);
        }
    }
    
    int ACCFontSizeAdjust(int _fontsize,
                          size_t _vidRatioX, size_t _vidRatioY,
                          size_t _playerX,   size_t _playerY,
                          size_t _canvasX,   size_t _canvasY){
        double videoRatio = (double)_vidRatioX / (double)_vidRatioY;
        double playerRatio= (double)_playerX / _playerY;
        if (playerRatio < videoRatio){
            return _fontsize * (double)_canvasX / (double)_playerX;
        }else{
            return _fontsize * (double)_canvasY / (double)_playerY;
        }
    }
}

#endif
