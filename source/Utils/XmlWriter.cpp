// xmlwriter.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "xmlwriter.h"
#include <stdarg.h>

#include "../Arch/ArchFile.h"


XmlWriter::XmlWriter(string sTmp)
{
    sXmlFile = sTmp;
    fp = NULL;
    iLevel = 0;
    fp = archFileOpen(sXmlFile.c_str(),"w");
    if(fp == NULL)
    {
        printf("Unable to open output file\n");
        return;
    }
    else
    {
        fprintf(fp,"<?xml version=\"1.0\" encoding=\"UTF-8\" \?>");
    }
}

XmlWriter::~XmlWriter()
{
    if(fp != NULL)
        fclose(fp);
    vectAttrData.clear();
}

void XmlWriter::CreateTag(string sTag, bool bClose)
{
    fprintf(fp,"\r\n");
    //Indent properly
    for(int iTmp =0;iTmp<iLevel;iTmp++)
        fprintf(fp,"    ");
    fprintf(fp,"<%s",sTag.c_str());
    //Add Attributes
    while(0 < vectAttrData.size()/2)
    {
        string sTmp = vectAttrData.back();
        fprintf(fp," %s=", sTmp.c_str());
        vectAttrData.pop_back();
        sTmp = vectAttrData.back();
        fprintf(fp,"\"%s\"", sTmp.c_str());
        vectAttrData.pop_back();
    }
    vectAttrData.clear();
    if( bClose ) {
        fprintf(fp,"/>");
    }else{
        fprintf(fp,">");
        sTagStack.push(sTag);
        iLevel++;
    }

}

void XmlWriter::CloseLastTag()
{
    fprintf(fp,"\r\n");
    iLevel--;
    //Indent properly
    for(int iTmp =0;iTmp<iLevel;iTmp++)
        fprintf(fp,"    ");
    fprintf(fp,"</%s>",sTagStack.top().c_str());
    sTagStack.pop();//pop out the last tag
    return;
}

void XmlWriter::CloseAllTags()
{
    while(sTagStack.size() != 0)
    {
       fprintf(fp,"\r\n");
       iLevel--;
        //Indent properly
       for(int iTmp =0;iTmp<iLevel;iTmp++)
           fprintf(fp,"    ");
       fprintf(fp,"</%s>",sTagStack.top().c_str());
       sTagStack.pop();//pop out the last tag
    }
    return;
}
void XmlWriter::CreateChild(string sTag,string sValue)
{
    fprintf(fp,"\r\n");
    //Indent properly
    for(int iTmp =0;iTmp<iLevel;iTmp++)
        fprintf(fp,"    ");
    fprintf(fp,"<%s",sTag.c_str());
    //Add Attributes
    while(0 < vectAttrData.size()/2)
    {
        string sTmp = vectAttrData.back();
        fprintf(fp," %s=", sTmp.c_str());
        vectAttrData.pop_back();
        sTmp = vectAttrData.back();
        fprintf(fp,"\"%s\"", sTmp.c_str());
        vectAttrData.pop_back();
    }
    vectAttrData.clear();
    //add value and close tag
    fprintf(fp,">%s</%s>",sValue.c_str(),sTag.c_str());
}

void XmlWriter::AddAtributes(string sKey, string sVal)
{
    vectAttrData.push_back(sVal);
    vectAttrData.push_back(sKey);
}


void XmlWriter::AddComment(string sComment)
{
    fprintf(fp,"\r\n");
    //Indent properly
    for(int iTmp =0;iTmp<iLevel;iTmp++)
        fprintf(fp,"    ");
    fprintf(fp,"<!--%s-->",sComment.c_str());
}
