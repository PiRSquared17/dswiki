#include "Markup.h"

#include <PA9.h>
#include <string>
#include <vector>
#include "char_convert.h"
#include "main.h"
#include "Globals.h"
#include "PercentIndicator.h"
#include "Statusbar.h"
#include "WIKI2XML.h"
#include "WIKI2XML_global.h"
#include "tinyxml.h"

using namespace std;


Markup::Markup()
{
	_td = NULL;
	TiXmlBase::SetCondenseWhiteSpace( false );
}


Markup::~Markup()
{
	if (_td)
	{
		delete _td;
		_td = NULL;
	}
}


void Markup::parse(string & Str)
{
	// Transform the wikimarkup-string into proper xml-markup
	WIKI2XML* w2x = new WIKI2XML();
	if (w2x)
	{
		w2x->setGlobals(_globals);
		// Transform
		w2x->parse(Str);
		// Str was modified, so we can delete this helper class
		delete w2x;
		w2x = NULL;
	}

	// Parse the xml-markup with tinyXML
	if (_td)
	{
		delete _td;
		_td = NULL;
	}
	_loadOK = false;
	_td = new TiXmlDocument();
	_td->ClearError();
	_globals->getStatusbar()->display("Parsing XML");
	PA_ClearTextBg(0);
	PA_ClearTextBg(1);
	PA_Clear16bitBg(0);
	PA_Clear16bitBg(1);
	struct mallinfo info = mallinfo();
	PA_OutputText(1,0, 0,"Heap size      : %d bytes   ", info.arena   ); /* total space allocated from system */
	PA_OutputText(1,0, 1,"Memory in use  : %d bytes   ", info.usmblks + info.uordblks);
	PA_OutputText(1,0, 2,"Memory in free : %d bytes   ", info.fsmblks + info.fordblks);
	int vorher = getFreeRAM();
	PA_OutputText(1,0, 3,"getFreeRAM     : %d bytes   ", vorher);

	_td->Parse(Str.c_str(), NULL, TIXML_ENCODING_UTF8);

	struct mallinfo info2 = mallinfo();
	PA_OutputText(1,0, 5,"Heap size      : %d bytes   ", info2.arena   ); /* total space allocated from system */
	PA_OutputText(1,0, 6,"Memory in use  : %d bytes   ", info2.usmblks + info2.uordblks);
	PA_OutputText(1,0, 7,"Memory in free : %d bytes   ", info2.fsmblks + info2.fordblks);
	int nachher = getFreeRAM();
	PA_OutputText(1,0, 8,"getFreeRAM     : %d bytes   ", nachher);

	PA_OutputText(1,0, 10,"Differenz (arena)     : %d", info2.arena-info.arena);
	PA_OutputText(1,0, 11,"Differenz (getFreeRAM): %c1%d", vorher-nachher);

	_loadOK = !(_td->Error());

	if (_loadOK)
	{
		vector <TiXmlNode*> index;
		build_index(_td, index);
		for (int a=0;a<index.size();a++)
		{
			PA_OutputText(0,0,a+5,"%s: %s",index[a]->Value(),((TiXmlText*) index[a]->FirstChild())->Value());
		}
		string ersterText = ((TiXmlText*) _td->LastChild()->FirstChild())->Value();
		PA_OutputText(0,0,12,"(%d Zeichen)\n%s", ersterText.length(), ersterText.c_str());
	}
	exit(0);
}

void build_index(TiXmlNode* pParent, vector <TiXmlNode*> & index)
{
	if ( !pParent ) return;

	TiXmlNode* pChild;
	if (pParent->Type() == TiXmlNode::ELEMENT)
	{
		string value = pParent->Value();
		if (value.length() == 2 && value[0] == 'h' && value[1] >= '0' && value[1] <= '9')
		{
			index.push_back(pParent);
		}
	}
	for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
	{
		build_index( pChild, index );
	}
}

int dump_attribs_to_stdout(TiXmlElement* pElement, unsigned int indent)
{
	if ( !pElement ) return 0;

	TiXmlAttribute* pAttrib=pElement->FirstAttribute();
	int i=0;
	int ival;
	double dval;
	printf("\n");
	while (pAttrib)
	{
		printf( "%s: value=[%s]", pAttrib->Name(), pAttrib->Value());

		if (pAttrib->QueryIntValue(&ival)==TIXML_SUCCESS)    printf( " int=%d", ival);
		if (pAttrib->QueryDoubleValue(&dval)==TIXML_SUCCESS) printf( " d=%1.1f", dval);
		printf( "\n" );
		i++;
		pAttrib=pAttrib->Next();
	}
	return i;
}


void dump_to_stdout( TiXmlNode* pParent, unsigned int indent = 0 )
{
	if ( !pParent ) return;

	TiXmlNode* pChild;
	TiXmlText* pText;
	int t = pParent->Type();
	int num;

	PA_ClearTextBg(1);
	switch ( t )
	{
		case TiXmlNode::DOCUMENT:
			PA_OutputText(1,0,5,"Document");
			break;

		case TiXmlNode::ELEMENT:
			PA_OutputText(1,0,5,"Element [%s]", pParent->Value() );
			num = dump_attribs_to_stdout(pParent->ToElement(), indent+1);
			switch(num)
			{
				case 0:
					PA_OutputText(1,0,6, "(No attributes)");
					break;
				case 1:
					PA_OutputText(1,0,6, "1 attribute");
					break;
				default:
					PA_OutputText(1,0,6, "%d attributes", num);
					break;
			}
			break;

		case TiXmlNode::COMMENT:
			PA_OutputText(1,0,5, "Comment: [%s]", pParent->Value());
			break;

		case TiXmlNode::UNKNOWN:
			PA_OutputText(1,0,5, "Unknown" );
			break;

		case TiXmlNode::TEXT:
			pText = pParent->ToText();
			PA_OutputText(1,0,5, "Text: [%s]", pText->Value() );
			break;

		case TiXmlNode::DECLARATION:
			PA_OutputText(1,0,5, "Declaration" );
			break;
		default:
			break;
	}
	PA_Sleep(30);

	for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling())
	{
		dump_to_stdout( pChild, indent+1 );
	}
}


bool Markup::LoadOK()
{
	return _loadOK;
}

void Markup::setGlobals(Globals* globals)
{
	_globals = globals;
}