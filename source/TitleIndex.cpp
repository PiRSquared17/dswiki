#include <PA9.h>
#include <fat.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "TitleIndex.h"

typedef struct
{
	char languageCode[2];
	u32 numberOfArticles;
	fpos_t	titlesPos;
	fpos_t	indexPos_0;
	fpos_t	indexPos_1;
	unsigned char version;
	char reserved1[1];
	char imageNamespace[32];
	char templateNamespace[32];
	char reserved2[160];
} FILEHEADER;


TitleIndex::TitleIndex(char* nameOfDataFile, char* nameOfIndexFile)
{
	_imageNamespace = (char*) "";
	_templateNamespace = (char*) "";
	isChinese = false;

	_dataFileName = nameOfDataFile;
	_indexFileName = nameOfIndexFile;

	_numberOfArticles = -1;

	_indexPos_0 = 0;
	_indexPos_1 = 0;

	_imageNamespace = (char*) "";
	_templateNamespace = (char*) "";

	// get the number of articles; this also checks if the files exists
	FILE* f = fopen(_dataFileName, "rb");

	if ( f )
	{
		int error = 0;

		FILEHEADER fileheader;

		// if the old, short fileheader is used this will read over the end of the header
		// no problem, if the file is less than 256 it's unuseable anyway
		fread(&fileheader.languageCode,      2, 1, f);
		fread(&fileheader.numberOfArticles,  4, 1, f);
		fread(&fileheader.titlesPos,         8, 1, f);
		fread(&fileheader.indexPos_0,        8, 1, f);
		fread(&fileheader.indexPos_1,        8, 1, f);
		fread(&fileheader.version,           1, 1, f);
		fread(&fileheader.reserved1,         1, 1, f);
		fread(&fileheader.imageNamespace,    32, 1, f);
		fread(&fileheader.templateNamespace, 32, 1, f);
		fread(&fileheader.reserved2,         160, 1, f);

		if ( !error )
		{
			_numberOfArticles = fileheader.numberOfArticles;
			_indexPos_0 = fileheader.indexPos_0 - fileheader.titlesPos;

			if ( fileheader.version==1 )
			{
				_indexPos_1 = fileheader.indexPos_1 - fileheader.titlesPos;
				_imageNamespace = fileheader.imageNamespace;
				_templateNamespace = fileheader.templateNamespace;
			}

			_titlesPos = 0x0;
		}

		fclose(f);
	}
}

TitleIndex::~TitleIndex()
{
}

char* TitleIndex::GetTitle(FILE* f, int articleNumber, int indexNo)
{
	_lastBlockPos = 0;
	_lastArticlePos = 0;
	_lastArticleLength = 0;

	if ( !f || articleNumber<0 || articleNumber>=_numberOfArticles  )
		return (char*) "";

	int indexPos = _indexPos_0;
	if ( indexNo==1 && _indexPos_1 )
		indexPos = _indexPos_1;

	int error = fseek(f, indexPos + articleNumber*sizeof(int), SEEK_SET);
	if ( error )
		return "";

	int titlePos;
	size_t read = fread(&titlePos, sizeof(int), 1, f);

	if ( !read )
		return "";

	error = fseek(f, _titlesPos + titlePos, SEEK_SET);
	if ( error )
		return "";


	// store the article location and size for use in the future
	fread(&_lastBlockPos, 8, 1, f);
	fread(&_lastArticlePos, 4, 1, f);
	fread(&_lastArticleLength, 4, 1, f);

	char result[1000] = "";

	char c = 0;
	int i = 0;

	while ((c=fgetc(f))) {
		result[i++]=c;
	}
	result[i]='\0';

	char* resptr = result;
	return resptr;
}

ArticleSearchResult* TitleIndex::FindArticle(char* title, bool multiple)
{
	if ( _numberOfArticles<=0  )
		return NULL;

	FILE* f = fopen(_indexFileName, "rb");
	if ( !f )
		return NULL;

	char lowercaseTitle[1000] = "";
	PA_CopyText(lowercaseTitle, title);
	strlwr(lowercaseTitle);

	int indexNo = 0;
	int foundAt = -1;
	int lBound = 0;
	int uBound = _numberOfArticles - 1;
	int index = 0;

	while ( lBound<=uBound )
	{
		index = (lBound + uBound) >> 1;

		// get the title at the specific index
		char* titleAtIndex = GetTitle(f, index, indexNo);

		// make it lowercase and skip the prefix
		strlwr(titleAtIndex); // TODO

		if ( strcmp(lowercaseTitle,titleAtIndex)<0 )
			uBound = index - 1;
		else if ( strcmp(lowercaseTitle,titleAtIndex)>0 )
			lBound = ++index;
		else
		{
			foundAt = index;
			break;
		}
	}

	if ( foundAt<0 )
	{
		fclose(f);
		return NULL;
	}

	// check if there are more than one articles with the same lowercase name
	int startIndex = foundAt;
	while ( startIndex>0 )
	{
		char* titleAtIndex = GetTitle(f, startIndex-1, indexNo);
		strlwr(titleAtIndex); // TODO

		if ( strcmp(lowercaseTitle,titleAtIndex)!=0 )
			break;

		startIndex--;
	}

	int endIndex = foundAt;
	while ( endIndex<(_numberOfArticles-1) )
	{
		char* titleAtIndex = GetTitle(f, endIndex+1, indexNo);
		strlwr(titleAtIndex); // TODO

		if ( strcmp(lowercaseTitle,titleAtIndex)!=0 )
			break;

		endIndex++;
	}

	if ( !multiple )
	{
		// return a result only if we have a direct hit (case is taken into account)

		if ( startIndex!=endIndex )
		{
			// check if one matches 100%
			for(int i=startIndex; i<=endIndex; i++)
			{
				char* titleInArchive = GetTitle(f, i, indexNo);
				if ( strcmp(title,titleInArchive)==0 )
				{
					fclose(f);

					return new ArticleSearchResult(title, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
				}
			}

			// nope, multiple matches
			fclose(f);

			return NULL;
		}
		else
		{
			// return the one and only result
			char* titleInArchive = GetTitle(f, foundAt, indexNo);
			fclose(f);

			return new ArticleSearchResult(title, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
		}
	}
	else
	{
		ArticleSearchResult* result = NULL;
		for(int i=startIndex; i<=endIndex; i++)
		{
			char* titleInArchive = GetTitle(f, i, indexNo);

			if ( strcmp(title,titleInArchive)==0 )
			{
				// 100% match
				DeleteSearchResult(result);

				fclose(f);

				return new ArticleSearchResult(title, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
			}

			// collect the results
			if ( !result )
				result = new ArticleSearchResult(title, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
			else
				result->Next = new ArticleSearchResult(title, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
		}

		fclose(f);

		return result;
	}
	return NULL;
}

char* TitleIndex::GetSuggestions(char* phrase, int maxSuggestions)
{
	char suggestions[1000] = "";

	if ( _numberOfArticles<=0  )
		return suggestions;

	FILE* f = fopen(_indexFileName, "rb");
	if ( !f )
		return suggestions;

	int indexNo = 1;
	if ( !_indexPos_1 )
		indexNo = 0;

	char lowercasePhrase[1000] = "";
	strcpy(lowercasePhrase,phrase);
	strlwr(lowercasePhrase); // TODO

	int phraseLength = strlen(phrase);
	if ( phraseLength==0 )
		return suggestions;

	int foundAt = -1;
	int lBound = 0;
	int uBound = _numberOfArticles - 1;
	int index = 0;
	char* titleAtIndex;

	while ( lBound<=uBound )
	{
		index = (lBound + uBound) >> 1;

		// get the title at the specific index
		titleAtIndex = GetTitle(f, index, indexNo);
		strlwr(titleAtIndex); // TODO

		if ( strcmp(lowercasePhrase,titleAtIndex) < 0)
			uBound = index - 1;
		else if ( strcmp(lowercasePhrase,titleAtIndex) > 0 )
			lBound = index + 1;
		else
		{
			foundAt = index;
			break;
		}
	}

	if ( foundAt<0 ) // not found
	{
		if ( strlen(titleAtIndex) > phraseLength )
			titleAtIndex[phraseLength] = '\0';

		if ( strcmp(lowercasePhrase,titleAtIndex) > 0 )
		{
			// last one?
			if ( index==_numberOfArticles-1)
			{
				fclose(f);
				return suggestions;
			}

			// no
			index++;
			titleAtIndex = GetTitle(f, index, indexNo);

			strlwr(titleAtIndex); // TODO

			if ( strlen(titleAtIndex) > phraseLength )
				titleAtIndex[phraseLength] = '\0';

			if ( strcmp(lowercasePhrase,titleAtIndex)!=0 )
			{
				// still not starting with the phrase?
				fclose(f);
				return suggestions;
			}
		}
		else if ( strcmp(lowercasePhrase,titleAtIndex)<0 )
		{
			// first one?
			if ( index==0 )
			{
				fclose(f);
				return suggestions;
			}

			// no
			index--;
			titleAtIndex = GetTitle(f, index, indexNo);
			strlwr(titleAtIndex); // TODO

			if ( strlen(titleAtIndex) > phraseLength )
				titleAtIndex[phraseLength] = '\0';

			if ( strcmp(lowercasePhrase,titleAtIndex)!=0 )
			{
				// still not starting with the phrase?
				fclose(f);
				return suggestions;
			}
		}

		foundAt = index;
	}

	// go to the first article which starts with the phrase
	int startIndex = foundAt;
	while ( startIndex>0 )
	{
		titleAtIndex = GetTitle(f, startIndex-1, indexNo);
		strlwr(titleAtIndex); // TODO

		if ( strlen(titleAtIndex)>phraseLength )
			titleAtIndex[phraseLength] = '\0';

		if ( strcmp(lowercasePhrase,titleAtIndex)!=0 )
			break;

		startIndex--;
	}

	int results = 0;
	while ( startIndex<(_numberOfArticles-1) && results<maxSuggestions )
	{
		char* suggestion = GetTitle(f, startIndex, indexNo);

		PA_CopyText(titleAtIndex,suggestion);
		strlwr(titleAtIndex); // TODO

// 		if ( strlen(titleAtIndex)>phraseLength )
// 			titleAtIndex[phraseLength] = '\0';

// 		if ( strcmp(lowercasePhrase,titleAtIndex)!=0 )
// 			break;

		int tempLen = strlen(suggestions);
 		if ( tempLen > 0 )
		{
			suggestions[tempLen++] = '\n';
		}
		strncpy(&suggestions[tempLen],suggestion,strlen(suggestion)+1);


		startIndex++;
		results++;
	}

	if ( (results==maxSuggestions) && (startIndex<(_numberOfArticles+1)) )
	{
		// check if the next would also meet
		startIndex++;

		char* suggestion = GetTitle(f, startIndex, indexNo);
		strcpy(titleAtIndex,suggestion);
		strlwr(titleAtIndex);

		if ( strlen(titleAtIndex)>phraseLength )
			titleAtIndex[phraseLength] = '\0';

		// yes, add an empty line at the end of the list
// 		if ( strcmp(lowercasePhrase,titleAtIndex)==0 )
// 			suggestions += "\n";
	}

	fclose(f);

	return suggestions;
}

ArticleSearchResult* TitleIndex::isRedirect(char* markup)
{
	int markupLength = strlen(markup);
	char* redirectStelle;
	redirectStelle = strstr(markup,"#redirect");
	if (redirectStelle==NULL)
		redirectStelle = strstr(markup,"#REDIRECT");

	if (redirectStelle==NULL) {
		return NULL;
	}
	else
	{
		char redirectTitel[1000] = "";
		redirectStelle+=12;
		while (redirectStelle[-2]!='[' || redirectStelle[-1]!='[')
			redirectStelle++;
		int i = 0;
		while ((redirectStelle[i]!=']') || (redirectStelle[i+1]!=']'))
		{
			redirectTitel[i] = redirectStelle[i];
			i++;
		}
		redirectTitel[i] = '\0';
		return FindArticle(redirectTitel);
	}
}

ArticleSearchResult* TitleIndex::GetRandomArticle()
{
	if ( _numberOfArticles<=0  )
		return NULL;

	FILE* f = fopen(_indexFileName, "rb");

	if ( !f )
		return NULL;

	int indexNo = 0;

	int articleNo = PA_RandMax(_numberOfArticles-1);

	char* titleInArchive = GetTitle(f, articleNo, indexNo);
	fclose(f);

	return new ArticleSearchResult(titleInArchive, titleInArchive, _lastBlockPos, _lastArticlePos, _lastArticleLength);
}

void TitleIndex::DeleteSearchResult(ArticleSearchResult* articleSearchResult)
{
	while ( articleSearchResult )
	{
		ArticleSearchResult* help = articleSearchResult;
		articleSearchResult = articleSearchResult->Next;

		delete(help);
	}
}

char* TitleIndex::DataFileName()
{
	return _dataFileName;
}

char* TitleIndex::IndexFileName()
{
	return _indexFileName;
}

int TitleIndex::NumberOfArticles()
{
	return _numberOfArticles;
}

char* TitleIndex::ImageNamespace()
{
	return _imageNamespace;
}

char* TitleIndex::TemplateNamespace()
{
	return _templateNamespace;
}


ArticleSearchResult::ArticleSearchResult(char* title, char* titleInArchive, fpos_t blockPos, int articlePos, int articleLength)
{
	Next = NULL;

	PA_CopyText( _title, title );
	PA_CopyText( _titleInArchive, titleInArchive);

	_blockPos = blockPos;
	_articlePos = articlePos;
	_articleLength = articleLength;
}

char* ArticleSearchResult::Title()
{
	return _title;
}

char* ArticleSearchResult::TitleInArchive()
{
	return _titleInArchive;
}


fpos_t ArticleSearchResult::BlockPos()
{
	return _blockPos;
}

int ArticleSearchResult::ArticlePos()
{
	return _articlePos;
}

int ArticleSearchResult::ArticleLength()
{
	return _articleLength;
}
