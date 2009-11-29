#ifndef xmlwriter_h
#define xmlwriter_h

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <stack>

using namespace std;
typedef stack<string> StackStrings;

class XmlWriter{
public:
	XmlWriter(string sTmp);
	~XmlWriter();
	void CreateChild(string sTag,string sValue);
	void CreateTag(string sTag, bool bClose = false);
	void CloseLastTag();
	void CloseAllTags();
	void AddAtributes(string sAttrName, string sAttrvalue);
	void AddComment(string sComment);
private:
	string sXmlFile;
	vector<string> vectAttrData;
	FILE *fp;
	int iLevel;
	StackStrings sTagStack;
};

#endif // xmlwriter_h

