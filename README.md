## AssCompressor
### @CHOSX_K9 @RecursiveG

AssCompressor 致力于将Bilibili (http://www.bilibili.tv) 的.XML格式弹幕文件转化为更容易更多播放器支持的 .ASS 格式字幕文件。

---------------------------------------

### 安装AssCompressor

    执行 make 指令编译AssCompressorCLI
    -D_withMultiThread                  批量处理时使用多线程（需要C++11支持）
    -D_WIN32_Font                       精确定位弹幕针对win修改字体名称
    -D_withColorOutput                  AssCompressor在终端输出时，采用彩色输出
    *AssCompressor 依赖 libCURL , rapidXML, rapidJSON
    libCURL:  	   http://curl.haxx.se/libcurl/
    libRapidXML:   http://rapidxml.sourceforge.net/
    libRapidJSON:  https://github.com/miloyip/rapidjson

### AssCompressor 所使用的 XML 配设输入
    
    AssCompressor 运行时需要一个配设文件。每一个配设文件对应了一个单独的任务，因此在程序运行的时候AssCompressor会要求用户输入一个.XML配设文件的目录
    
    AssCompressor 配设XML的格式：
    
    <AssCompressor version="CLIv1">
        <XMLUri>[Bilibili URL]</XMLUri><!--B站XML路径 HTTP与本地文件均可-->
        <ExportLocation>[Export File Location]</ExportLocation><!--ASS文件输出路径，留空输出到stdout-->
        <!--ACC=精确定位 BOT=底部 TOP=顶部 ROL=滚动-->
        <!--是否输出对应类型的弹幕-->
	    <ACCEnable>[true | false]</ACCEnable>
	    <TOPEnable>[true | false]</TOPEnable>
	    <BOTEnable>[true | false]</BOTEnable>
	    <ROLEnable>[true | false]</ROLEnable>
	    <!--弹幕透明度，1=不透明，0=全透明-->
	    <AlphaTOP>[0-1 Double Value]</AlphaTOP>
	    <AlphaBOT>[0-1 Double Value]</AlphaBOT>
	    <AlphaROL>[0-1 Double Value]</AlphaROL>
	    <AlphaACC>[0-1 Double Value]</AlphaACC>
	    <!--重叠层次，越大堆叠在越上面-->
        <LayerTOP>[Long int Value]</LayerTOP>
        <LayerBOT>[Long int Value]</LayerBOT>
        <LayerROL>[Long int Value]</LayerROL>
        <LayerACC>[Long int Value]</LayerACC>  
        <!--视频真实分辨率，请至少保证长宽比正确，否则将出现字体拉伸现象-->
	    <DisplayX>[Long int Value]</DisplayX>
	    <DisplayY>[Long int Value]</DisplayY>
	    <!--默认字体名称和大小（滚动弹幕）-->
	    <DefaultFontName>[String Value]</DefaultFontName>
	    <DefaultFontSize>[Long int Value]</DefaultFontSize>
	    <MovingVelocity>[0-2 Double Value]</MovingVelocity><!--滚动弹幕运动速度-->
	    <DynamicCompare>[0-1 Double Value]</DynamicCompare><!---->
	    <MultilineComment>[true | false]</MultilineComment><!--是否将滚动弹幕中的'\n'解释成换行-->
        <MultilineCompare>[true | false]</MultilineCompare>
        <!--是否使用多行高度计算滚动弹幕碰撞体积,仅当MultilineComment为'true'时有效-->
        <Bold>[true | false]</Bold><!--是否加粗字体-->
        <!--以下设置只影响精确定位弹幕的现实效果，如果没有精确定位弹幕或是关闭了精确定位弹幕，请保持默认-->
        <!--B站播放器定位框大小 X:524 Y:386-->
	    <iframeX>[Long int Value]</iframeX>
	    <iframeY>[Long int Value]</iframeY>
	    <!--精确定位弹幕是否跟随指定字体-->
	    <FollowACCFontName>[true | false]</FollowACCFontName>
        <VideoRatio>[16:9 | 4:3]</VideoRatio><!--视频长宽比-->
        <!--以下为实验性功能，存在BUG，会影响展示效果-->
        <ACCAutoAdjust>[true | false]</ACCAutoAdjust><!--将黑边上的精确定位弹幕移到视频上-->
        <ACCFixRotate>[true | false]</ACCFixRotate><!--模拟透视效果-->
        <!--以下是可选项目-->
		<RemoveKeyword>[String Value]</RemoveKeyword> <!--按照关键词过滤弹幕内容-->
		<RemoveColor>[String Value]</RemoveColor><!--按照颜色过滤弹幕-->
		<RemoveUser>[String Value]</RemoveUser><!--屏蔽指定用户ID弹幕-->
		<RemovePool>[String Value]</RemovePool><!--屏蔽指定弹幕池弹幕-->
		<AllowVisitorComment>[true | false]</AllowVisitorComment><!--是否显示访客弹幕-->
        <History>[Unsigned Long int Value]</History> <!--时间戳，获取指定时间戳之前的弹幕-->
    </AssCompressor>
    
    ＊此.XML配设格式文件位于doc文件夹中
    
### 联系我以及错误报告
    CHOSX_K9 : chenhongji2011@me.com

### Notes
	现在处于代码重构的状态中额。。。希望可以按时完成重构计划。。之前的代码会放在一个old的branch里。。。。
