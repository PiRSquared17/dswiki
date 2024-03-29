#ifndef _WIKIMARKUPGETTER_H
#define _WIKIMARKUPGETTER_H

#include <PA9.h>
#include <fat.h>
#include <vector>
#include <string>
#include "main.h"

using namespace std;

class TitleIndex;
class Globals;

class WikiMarkupGetter
{
public:
	WikiMarkupGetter();
	~WikiMarkupGetter();

	void load(string basename, bool internal = false);

	void   getMarkup(string & markup, string title);
	string GetLastArticleTitle();

	void setGlobals(Globals* globals);
private:
	vector<FILE*>  _filepointers;
	vector<fpos_t> _filesizes;
	vector<u64>    _file_absoluteEnds;
	string         _lastArticleTitle;
	Globals* _globals;
};

#endif
