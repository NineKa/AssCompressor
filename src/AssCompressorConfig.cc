#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <iterator>
#include <map>
#include <utility>
#include "assUtil.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidjson/document.h"

#ifdef CXX11REGEX
	#include <regex>
#else
	#include <boost/regex.hpp>
#endif

/*  +---------------------------+-------------------+
	|	  Configure Flag		| 		Type		|
	+---------------------------+-------------------+
	| -aid 						| [size_t]			|
	| -cid 						| [size_t]			|
	| -webpageURL 				| [const char*]		|
	| -commentPageURL 			| [const char*]		|
	| -rollBack 				| [time_t]			|
	| -canvasX 					| [size_t]			|
	| -canvasY 					| [size_t]			|
	| -duration 				| [double]			|
	| -playerX 					| [size_t]			|
	| -palyerY 					| [size_t]			|
	| -vidRatioX 				| [size_t]			|
	| -vidRatioY 				| [size_t]			|
	| -defaultFontName 			| [const char*]		|
	| -defaultFontSize 			| [size_t]			|
	| -layerRollType 			| [int]				|
	| -layerFixType 			| [int]				|
	| -layerACCType 			| [int]				|
	| -setSubtitleTop 			| [true | false]	|
	| -multiLineCollision 		| [true | false]	|
	| -multiLineComment 		| [true | false]	|
	| -borderSize 				| [double]			|
	| -fadeScalarDefult 		| [double]			|
	| -fadeScalarTopRoll 		| [double]			|
	| -fadeScalarBottomRoll		| [double]			|
	| -fadeScalarTopFix 		| [double]			|
	| -fadeScalarReverseTopRoll | [double]			|
	| -fadeScalarFX 			| [double]			|
	| -fadeScalarCode 			| [double]			|
	| -forceType 				| [const char*]		|
	| -forceRawType 			| [const char*]		|
	| -forceFXStr 				| [const char*]		|
	+---------------------------+-------------------+ */

#define assCompressorConfigureError_NOTENOUGHROOM "assCompressorConfigure not enough room"

namespace assCompressorConfigure{
	typedef std::vector<std::string> configArrType;
	typedef enum {Tlong,Tcstr,Tdouble,Tbool,Tsignedlong} typeAssertT;
	typedef assUtil::assConfigure configureT;
	std::map<std::string, typeAssertT> typeMap;
	
	void assCompressorConfigureInit(){
		typedef std::map<std::string, typeAssertT>::value_type typeMapValueT;
		typeMap.insert(typeMapValueT("aid", typeAssertT::Tlong));
		typeMap.insert(typeMapValueT("cid", typeAssertT::Tlong));
		typeMap.insert(typeMapValueT("webpageURL", typeAssertT::Tcstr));
		typeMap.insert(typeMapValueT("commentPageURL", typeAssertT::Tcstr));
		typeMap.insert(typeMapValueT("rollBack", typeAssertT::Tlong));
		typeMap.insert(typeMapValueT("canvasX", typeAssertT::Tlong));
		typeMap.insert(typeMapValueT("canvasY", typeAssertT::Tlong));
		typeMap.insert(typeMapValueT("duration", typeAssertT::Tdouble));
		typeMap.insert(typeMapValueT("playerX", typeAssertT::Tlong));
		typeMap.insert(typeMapValueT("playerY", typeAssertT::Tlong));
		typeMap.insert(typeMapValueT("vidRatioX", typeAssertT::Tlong));
		typeMap.insert(typeMapValueT("vidRatioY", typeAssertT::Tlong));
		typeMap.insert(typeMapValueT("defaultFontName", typeAssertT::Tcstr));
		typeMap.insert(typeMapValueT("defaultFontSize", typeAssertT::Tlong));
		typeMap.insert(typeMapValueT("layerRollType", typeAssertT::Tsignedlong));
		typeMap.insert(typeMapValueT("layerFixType", typeAssertT::Tsignedlong));
		typeMap.insert(typeMapValueT("layerACCType", typeAssertT::Tsignedlong));
		typeMap.insert(typeMapValueT("setSubtitleTop", typeAssertT::Tbool));
		typeMap.insert(typeMapValueT("multiLineCollision", typeAssertT::Tbool));
		typeMap.insert(typeMapValueT("multiLineComment", typeAssertT::Tbool));
		typeMap.insert(typeMapValueT("borderSize", typeAssertT::Tdouble));
		typeMap.insert(typeMapValueT("fadeScalarDefult", typeAssertT::Tdouble));
		typeMap.insert(typeMapValueT("fadeScalarTopRoll", typeAssertT::Tdouble));
		typeMap.insert(typeMapValueT("fadeScalarBottomRoll", typeAssertT::Tdouble));
		typeMap.insert(typeMapValueT("fadeScalarTopFix", typeAssertT::Tdouble));
		typeMap.insert(typeMapValueT("fadeScalarReverseTopRoll", typeAssertT::Tdouble));
		typeMap.insert(typeMapValueT("fadeScalarFX", typeAssertT::Tdouble));
		typeMap.insert(typeMapValueT("fadeScalarCode", typeAssertT::Tdouble));
		typeMap.insert(typeMapValueT("forceType", typeAssertT::Tcstr));
		typeMap.insert(typeMapValueT("forceRawType", typeAssertT::Tcstr));
		typeMap.insert(typeMapValueT("forceFXStr", typeAssertT::Tcstr));
	}
	const char* longRegex = "[0-9]+";
	bool regexMatch(const char* _str, const char* _expr){
	#ifdef CXX11REGEX
		std::string str(_str);
		std::regex regexExpr(_expr);
		return std::regex_match(str, regexExpr);
	#else
		std::string str(_str);
		boost::regex regexExpr(_expr);
		return boost::regex_match(str, regexExpr);
	#endif
	}
	int strcmpNCase(const char* _str1, const char* _str2){
		char* _str1CPY = (char*)malloc(sizeof(char)*strlen(_str1));
		char* _str2CPY = (char*)malloc(sizeof(char)*strlen(_str2));
		if (_str1CPY == nullptr) throw std::runtime_error(assCompressorConfigureError_NOTENOUGHROOM);
		if (_str2CPY == nullptr) throw std::runtime_error(assCompressorConfigureError_NOTENOUGHROOM);
		memset(_str1CPY, '\0', sizeof(char)*strlen(_str1));
		memset(_str2CPY, '\0', sizeof(char)*strlen(_str2));
		const size_t lengthStr1 = strlen(_str1);
		const size_t lengthStr2 = strlen(_str2);
		for (size_t iter = 0; iter < lengthStr1; iter++){
			_str1CPY[iter] = (char)toupper((int)_str1[iter]);
		}
		for (size_t iter = 0; iter < lengthStr2; iter++){
			_str2CPY[iter] = (char)toupper((int)_str2[iter]);
		}
		int result = strcmp(_str1CPY, _str2CPY);
		free(_str1CPY);
		free(_str2CPY);
		return result;
	}
}

using namespace assCompressorConfigure;
int main (int argc, char const *argv[]){
	assCompressorConfigureInit();
	std::cout<< strcmpNCase("asfasf","ASF") <<std::endl;
	std::cout<< regexMatch("123", longRegex) << std::endl;
	return 0;
}