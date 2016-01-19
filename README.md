# AssCompressor
AssCompressor 致力于将Bilibili (http://www.bilibili.tv) 的.XML格式弹幕文件转化为更容易更多播放器支持的 .ASS 格式字幕文件。

### Compile

在编译之前需要填写位于./src/configure.h中

    bilibiliWebManager_DedeUserID
    bilibiliWebManager_SESSDATA
    bilibiliWebManager_AppKey
    
其中DedeUserID和SESSDATA为AssCompressor登录bilibili服务器时所使用的用户Cookie，AppKey为请求bilibili interface API 是所使用的AppKey。

DedeUserID以及SESSDATA为可选值，但必须提供一个有效的AppKey。

AssCompressor基于libcurl,boost,rapidxml,rapidjson.

### Input
    <AssCompressor>
	    <!-- 1 --><aid>[size_t]</aid> <index>[size_t]</index>
	    <!-- 2 --><cid>[size_t]</cid>
	    <!-- 3 --><webpageURL>[const char*]</webpageURL>
	    <!-- 4 --><commentPageURL>[const char*]</commentPageURL>

	    <rollBack>[time_t]</rollBack>

	    <canvasX>[size_t]</canvasX>
	    <canvasY>[size_t]</canvasY>
	    <duration>[double]</duration>
	    <playerX>[size_t]</playerX>
	    <playerY>[size_t]</playerY>
	    <vidRatioX>[size_t]</vidRatioX>
	    <vidRatioY>[size_t]</vidRatioY>

	    <defaultFontName>[const char*]</defaultFontName>
	    <defaultFontSize>[size_t]</defaultFontSize>

	    <layerRollType>[int]</layerRollType>
    	<layerFixType>[int]</layerFixType>
    	<layerACCType>[int]</layerACCType>
    	<setSubtitleTop> true | false </setSubtitleTop>
    	<multiLineCollision> true | false </multiLineCollision>
    	<multiLineComment> true | false </multiLineComment>

    	<borderSize>[double]</borderSize>
    	<fadeScalar>
		[[double],[double],[double],[double],[double],[double],[double],[double],[double]]
    	</fadeScalar>

    	<forceType>[[const char*], ... ]</forceType>
    	<forceRawType>[[const char*], ...]</forceRawType>
    	<forceFXStr>[[const char*], ...]</forceFXStr>
    </AssCompressor>
    
其中fadeScalar和forceType, forceRawType, forceFXStr为有效的JSON数组。
