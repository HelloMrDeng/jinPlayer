/*******************************************************************************
	File:		CLangText.h

	Contains:	the language text header file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-21		Fenger			Create file

*******************************************************************************/
#ifndef __CLangText_H__
#define __CLangText_H__

#include "CBaseObject.h"
#include "CNodeList.h"

#define YYLANG_ENG		0
#define YYLANG_CHN		1

#define	YYTEXT_Player		1   // surePlayer
#define	YYTEXT_Error		2	// Error
#define	YYTEXT_Info			3   // Info
#define	YYTEXT_DelFile		4   // Delete File
#define	YYTEXT_FileInfo		5   // File Info
#define	YYTEXT_Create		6   // Create
#define	YYTEXT_PlayFail		20  // Start to play failed!
#define	YYTEXT_OpenFail		21  // Open file failed!
#define	YYTEXT_FileExist	22  // The file exist. Are you sure replace it?
#define	YYTEXT_WriteFail	23  // Write file failed!
#define	YYTEXT_PW_6			24  // The Password number was bot 6!
#define	YYTEXT_PW_Same		25  // The two password are not same!
#define	YYTEXT_CharNum		26  // The Password should be char or number only!
#define	YYTEXT_PW_Exist		27  // The Password had already existed!
#define	YYTEXT_FindContent	28  // It can't find the content!
#define	YYTEXT_DeleteFile	29  // Are you sure delete the file?
#define	YYTEXT_OpenReturn	30  // Open file %s failed! return 0X%08X
#define	YYTEXT_CreateBox	32  // Please create new security box with Add New Box.
#define	YYTEXT_OpenBox		33  // Open your security box now.
#define	YYTEXT_FolderExist	34  // The folder was already exist!
#define	YYTEXT_PWLimit		35  // The password must be Digital or Character and number is six.
#define	YYTEXT_Password		51  // Password
#define	YYTEXT_Confirm		52  // Confirm
#define	YYTEXT_Folder		53  // Folder
#define	YYTEXT_Open			54  // Open
#define	YYTEXT_Add			55  // Add
#define	YYTEXT_Delete		56  // Delete
#define	YYTEXT_OK			57  // OK
#define	YYTEXT_Cancel		58  // Cancel
#define	YYTEXT_MyBox		59  // MyBox
#define	YYTEXT_OpenURL		60  // Open URL ...
#define	YYTEXT_CopyFile		61  // Copy File ...
#define	YYTEXT_BackUpFolder	62  // Back Up Folder
#define	YYTEXT_NewFile		63  // New File
#define	YYTEXT_NewFolder	64  // New Folder
#define	YYTEXT_Track		65  // Track
#define	YYTEXT_DelOrgFile	66  // Delete Original File
#define	YYTEXT_CloseFinish	67  // Close Finished
#define	YYTEXT_Start		68  // Start
#define	YYTEXT_ExportFile	69  // Export file from box
#define	YYTEXT_AboutPlayer	70  // About jinPlayer
#define	YYTEXT_ExitPlayer	71  // exit jinPlayer
#define	YYTEXT_CheckUpate	72  // Check jinPlayer update
#define	YYTEXT_GetInfoFail	73  // It can't get update info.
#define	YYTEXT_DL_Fail		74  // It downloads file failed.
#define	YYTEXT_LastVer		75  // The current ls lastest version.
#define	YYTEXT_StartUPFail	76  // Start update exe file failed.

#define yyLangGetText		CLangText::g_pLang->GetText

class CLangText : public CBaseObject
{
public:
struct TEXT_Item {
	int			nID;
	TCHAR *		pText;
	TEXT_Item() {
		nID = -1;
		pText = NULL;
	}
};
static CLangText *	g_pLang;

public:
	CLangText(void * hInst);
	virtual ~CLangText(void);

	virtual int		GetLang (void);
	virtual int		setLang (int nLang);

	virtual TCHAR *	GetText (int nID);

protected:
	virtual bool	ReadText (void);
	virtual bool	FreeText (void);

protected:
	void *				m_hInst;
	int					m_nLang;

	TCHAR						m_szError[32];
	CObjectList<TEXT_Item>		m_lstText;
};

#endif // __CLangText_H__
