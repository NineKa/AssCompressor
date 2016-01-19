#ifndef configure_file
#define configure_file

#define ASSCOMPRESSOR_Signature "AssCompressor"
/*==========================================================================*/
/*class netFetcher                                                          */
/*==========================================================================*/

#define quickFetch_ConnectTimeoutMS     3000L
#define quickFetch_TimeoutMS            7000L
#define quickFetch_FollowRedirection    true
#define quickFetch_UserAgent            "AssCompressorbot"

/*==========================================================================*/


/*==========================================================================*/
/*class bilibiliWebManager                                                  */
/*==========================================================================*/

#define bilibiliWebManager_DedeUserID  [DedeUserID]
#define bilibiliWebManager_SESSDATA    [SESSDATA]
#define bilibiliWebManager_AppKey      [AppKey]

//#define bilibiliWebManager_FetchInterfaceOnConstruct
//#define bilibiliWebManager_FetchCommentOnConstruct
//#define bilibiliWebManager_FetchRollBackOnConstruc
//#define CXX11REGEX

#define bilibiliWebManager_MAXBuffer   1024

#define bilibiliWebManager_CIDRegex    "cid=([0-9]*)"
#define bilibiliWebManager_AIDRegex    "aid=([0-9]*)"
#define bilibiliWebManager_TitleRegex  "<h1 title=\"([^\"]*)\">"
#define bilibiliWebManager_DesRegex    "name=\"description\" content=\"([^\"]*)\""
#define bilibiliWebManager_AuthorRegex "name=\"author\" content=\"([^\"]*)\""
#define bilibiliWebManager_KeywordRegex "name=\"keywords\" content=\"([^\"]*)\""

#define bilibiliWebManager_CommentURL  "http://comment.bilibili.com/%s.xml"
#define bilibiliWebManager_CommentRoll "http://comment.bilibili.com/rolldate,%s"
#define bilibiliWebManager_Interface   "http://interface.bilibili.com/playurl?appkey=%s&cid=%s"
#define bilibiliWebManager_URL         "http://www.bilibili.com/video/av%s/index_%s.html"

/*==========================================================================*/

/*==========================================================================*/
/*class bilibiliCommentContainer                                            */
/*==========================================================================*/

#define bilibiliCommentContainer_SupportTraceLine
#define bilibiliCommentContainer_WellFormAttribute  8

/*==========================================================================*/

/*==========================================================================*/
/*class assConvter                                                          */
/*==========================================================================*/

#define assConverter_MAXBuffer                      5120
#define assConverter_attributeConstructor           "%lf,%d,%d,%d,%lu,%d,%s,%d"
#define assConverter_ACC_14Constructor              "[%lu,%lu,\"%lf-%lf\",%lf,\"%s\",%d,%d,%lu,%lu,%lu,%lu,%s,\"%s\",0]"

#define assConverter_Append_Statistic
#define assConverter_Append_ForceList

#define BilibiliFont_KEY_HEI      "黑体"
#define BilibiliFont_KEY_YouYuan  "幼圆"
#define BilibiliFont_KEY_Song     "宋体"
#define BilibiliFont_KEY_Kai      "楷体"
#define BilibiliFont_KEY_MS_HEI   "微软雅黑"
#define BilibiliFont_Name_HEI      "黑体-简 细体"
#define BilibiliFont_Name_YouYuan  "圆体-简 细体"
#define BilibiliFont_Name_Song     "宋体-简 常规体"
#define BilibiliFont_Name_Kai      "楷体-简 常规体"
#define BilibiliFont_Name_MS_HEI   "兰亭黑-简 纤黑"

#define BilibiliFont_Default_FontName  "兰亭黑-简 纤黑"
/*==========================================================================*/

#endif