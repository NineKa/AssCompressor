#include "bilibiliWebManager.h"
#include "colorize.h"
#include <tclap/CmdLine.h>
#include <iostream>
#include <string>
#include <vector>

#define bilibiliURLFetcher_Version "(Stable)"
#define bilibiliURLFetcher_Intro   "" \
    "bilibiliURLFetcher - bilibili Video URL Fetcher @K9"
using namespace std;

TCLAP::CmdLine argsHandler(bilibiliURLFetcher_Intro, ' ', bilibiliURLFetcher_Version, true);
TCLAP::ValueArg<std::string> urlArg("u", "url", "Specify bilibili URL", false, "", "string");
TCLAP::ValueArg<size_t> aidArg("a", "aid", "Specify bilibili AID", false, 0L, "unsigned integer");
TCLAP::ValueArg<size_t> indexArg("i", "index", "Specify bilibili Video Index", false, 1L, "unsigned integer");

int main(int argc, char *argv[]) {
    argsHandler.add(urlArg);
    argsHandler.add(indexArg);
    argsHandler.add(aidArg);
    argsHandler.parse(argc, argv);
    bool hasUrl = !(urlArg.getValue().empty());
    bool hasAid = !(aidArg.getValue() == 0L);
    if ((!hasUrl) && (!hasAid)) {
	TCLAP::StdOutput printer;
	printer.usage(argsHandler);
	return 0;
    }
    bilibiliWebManager* handler = nullptr;
    if (hasUrl) handler = new bilibiliWebManager(urlArg.getValue());
    if (hasAid) {
	if (handler != nullptr) {
	    TCLAP::StdOutput printer;
	    printer.usage(argsHandler);
	    return 0;
	}
	std::string url = bilibiliWebManager::bilibiliURL(aidArg.getValue(), indexArg.getValue());
	handler = new bilibiliWebManager(url);
    }
    handler->fetchInterfaceXML();
    handler->fetchCommentXML();
    std::cout<<colorize(std::string("Title      : "),ColorIO_Style::Highlight)<<handler->getTitle()<<std::endl;
    std::cout<<colorize(std::string("Author     : "),ColorIO_Style::Highlight)<<handler->getAuthor()<<std::endl;
    std::cout<<"  "<<handler->getDescription()<<std::endl;
    std::cout<<colorize(std::string("Comment XML: "),ColorIO_Style::Highlight)<<handler->getCommentURL()<<std::endl;
    std::cout<<colorize(std::string("Keywords   : "),ColorIO_Style::Highlight)<<
	bilibiliWebManager::getKeywordsString(handler->getKeywordsList())<<std::endl;
    std::cout<<colorize(std::string("Video Sections: "),ColorIO_Style::Highlight)<<std::endl;
    bilibiliWebManager::videoSectionList videoSec = handler->getVideoSectionList();
    for (bilibiliWebManager::videoSectionList::iterator iter = videoSec.begin();
	 iter != videoSec.end(); iter++) {
	std::cout<<colorize(std::string("Section "),ColorIO_Style::Highlight)<<iter->order<<
	    colorize(std::string(": "),ColorIO_Style::Highlight)<<std::endl;
	std::cout<<colorize(std::string("\tSize        : "),ColorIO_Style::Highlight)<<iter->size<<std::endl;
	std::cout<<colorize(std::string("\tDuration(s) : "),ColorIO_Style::Highlight)<<iter->duration<<std::endl;
	std::cout<<colorize(std::string("\tURL         : "),ColorIO_Style::Highlight)<<iter->url<<std::endl;
	if (!(iter->backup_url.empty())) {
	    std::cout<<colorize(std::string("\tBackup URLs : "),ColorIO_Style::Highlight)<<std::endl;
	    for (std::vector<std::string>::iterator iter2 = iter->backup_url.begin();
		 iter2 != iter->backup_url.end(); iter2++){
		std::cout<<"\t\t"<<*iter2<<std::endl;
	    }
	}
    }
    return 0;
}
