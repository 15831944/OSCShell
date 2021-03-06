#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <comdef.h>
#include <conio.h>
#include <atltime.h>
#include "Database.h"


ADODB::_RecordsetPtr rec1=NULL;

_variant_t  vtMissing1(DISP_E_PARAMNOTFOUND, VT_ERROR); 

void ErrorHandler(_com_error &e, char* ErrStr)
{
  
	sprintf(ErrStr,"Error:\n");
	sprintf(ErrStr,"%sCode = %08lx\n",ErrStr ,e.Error());
	sprintf(ErrStr,"%sCode meaning = %s\n", ErrStr, (char*) e.ErrorMessage());
	sprintf(ErrStr,"%sSource = %s\n", ErrStr, (char*) e.Source());
	sprintf(ErrStr,"%sDescription = %s",ErrStr, (char*) e.Description());
}
Database::~Database()
{
	if(isOpen)
	{
		m_Cnn->Close();
	}
}
Database::Database()
{
	isOpen=false;
	m_Cnn=NULL;
	sprintf(m_ErrStr,"NULL POINTER");
}

void Database::GetErrorErrStr(char* ErrStr)
{
	sprintf(ErrStr,"%s",m_ErrStr);
}

void Table::GetErrorErrStr(char* ErrStr)
{
	sprintf(ErrStr,"%s",m_ErrStr);
}

bool Database::Open(char* UserName, char* Pwd,char* CnnStr)
{
	//cnn->Open(strCnn,"sa","sa",NULL);
	try
	{
		HRESULT hr;
		hr    = m_Cnn.CreateInstance(__uuidof( ADODB::Connection));
		//m_Cnn->CommandTimeout=5;
		m_Cnn->Open(CnnStr, UserName, Pwd, NULL);
	}
	
	CATCHERROR(m_Cnn,0)
	
	isOpen=true;
	sprintf(m_ErrStr,"Success");
	return 1;
}

bool Database::OpenTbl(int Mode, char* CmdStr, Table *Tbl)
{
	if(m_Cnn==NULL)
	{
		Tbl->m_Rec=NULL;
		sprintf(m_ErrStr,"Invalid Connection");
		return 0;
	}
	RecPtr t_Rec=NULL;	
	try
	{
		//t_Rec->putref_ActiveConnection(m_Cnn);
		//vtMissing<<-->>_variant_t((IDispatch *) m_Cnn, true)
		t_Rec.CreateInstance( __uuidof( ADODB::Recordset ) );
		t_Rec->Open(CmdStr,_variant_t((IDispatch *) m_Cnn, true),ADODB::adOpenStatic,ADODB::adLockOptimistic,Mode);
	}
	
	CATCHERROR(Tbl->m_Rec,0)

	Tbl->m_Rec=t_Rec;
	sprintf(m_ErrStr,"Success");
	return 1;
}

bool Database::Execute(char* CmdStr)
{
	try
	{
		m_Cnn->Execute(CmdStr,NULL,1);
	}
	catch(_com_error &e)
	{
		ErrorHandler(e,m_ErrStr);
		return 0;
	}
	sprintf(m_ErrStr,"Success");
	return 1;
}

bool Database::Execute(char* CmdStr, Table *Tbl)
{
	RecPtr t_Rec=NULL;
	try
	{
		t_Rec=m_Cnn->Execute(CmdStr,NULL,1);
	}

	CATCHERROR(Tbl->m_Rec,0)

	sprintf(m_ErrStr,"Success");
	Tbl->m_Rec=t_Rec;
	sprintf(m_ErrStr,"Success");
	return 1;
}
Table::~Table()
{
	if(m_Rec!=NULL)
	{
		m_Rec->Close();
	}
}

Table::Table()
{
	m_Rec=NULL;
}

int Table::ISEOF()
{
	int rs;
	if(m_Rec==NULL)
	{
		sprintf(m_ErrStr,"Invalid Record");
		return -1;
	}
	try{
		rs=m_Rec->EndOfFile;
	}
	
	CATCHERROR(m_Rec,-2)

	sprintf(m_ErrStr,"Success");
	return rs;
}

bool Table::Get(char* FieldName, char* FieldValue)
{
	try
	{	
		int count=0,i=0;
		_variant_t  vtValue;
		VariantInit(&vtValue);
		vtValue = m_Rec->Fields->GetItem(FieldName)->GetValue();
		if (vtValue.vt==VT_DATE)
		{			
			SYSTEMTIME   systime; 
			VariantTimeToSystemTime(vtValue.date,&systime);
			int day = systime.wDay;
			int month = systime.wMonth;
			int year = systime.wYear;
			int hour = systime.wHour;
			int minutes = systime.wMinute;
			int second = systime.wSecond;
			sprintf(FieldValue,"%d-%02i-%02i %02i:%02i:%02i",year,month,day,hour,minutes,second);

/*
			count=vtValue.parray->rgsabound[0].cElements;
			unsigned char *p=(unsigned char *)vtValue.parray->pvData;
			char st[10];
			for(i=0;i<count;i++)
			{
			   sprintf(st,"%02x",p[i]);
			   strcat(FieldValue,st);
			}
*/
		}else
		if (vtValue.vt==VT_UINT||vtValue.vt==VT_INT||vtValue.vt==VT_DECIMAL||vtValue.vt==VT_I2||vtValue.vt==VT_I4)
		{		
			sprintf(FieldValue,"%d",vtValue.intVal);		
		}else
		{
			if(m_Rec->Fields->GetItem(FieldName)->GetActualSize()>0){
				sprintf(FieldValue,"%s",(LPCSTR)((_bstr_t)vtValue.bstrVal));
			}
		}
	}

	CATCHERRGET

	sprintf(m_ErrStr,"Success");
	return 1;
}
/*
bool Table::Get(char* FieldName, char* FieldValue)
{
	try
	{
		_variant_t  vtValue;
		vtValue = m_Rec->Fields->GetItem(FieldName)->GetValue();		
		sprintf(FieldValue,"%s",(LPCSTR)((_bstr_t)vtValue.bstrVal));
	}

	CATCHERRGET

	sprintf(m_ErrStr,"Success");
	return 1;
}
*/
bool Table::Get(char* FieldName,int& FieldValue)
{
	try
	{
		_variant_t  vtValue;
		vtValue = m_Rec->Fields->GetItem(FieldName)->GetValue();
		FieldValue=vtValue.intVal;
	}

	CATCHERRGET

	sprintf(m_ErrStr,"Success");
	return 1;
}

bool Table::Get(char* FieldName,float& FieldValue)
{
	try
	{
		_variant_t  vtValue;
		vtValue = m_Rec->Fields->GetItem(FieldName)->GetValue();
		FieldValue=vtValue.fltVal;
	}

	CATCHERRGET

	sprintf(m_ErrStr,"Success");
	return 1;
}

bool Table::Get(char* FieldName,double& FieldValue)
{
	try
	{
		_variant_t  vtValue;
		vtValue = m_Rec->Fields->GetItem(FieldName)->GetValue();
		FieldValue=vtValue.dblVal;
		//GetDec(vtValue,FieldValue,3);
	}

	CATCHERRGET

	sprintf(m_ErrStr,"Success");
	return 1;
}

HRESULT Table::MoveNext()
{
	HRESULT hr;
	try
	{
		hr=m_Rec->MoveNext();
	}
	catch(_com_error &e)
	{
		ErrorHandler(e,m_ErrStr);
		//m_Rec=NULL;
		return -2;
	}
	sprintf(m_ErrStr,"Success");
	return hr;
}

HRESULT Table::MovePrevious()
{
	HRESULT hr;
	try
	{
		hr=m_Rec->MovePrevious();
	}
	catch(_com_error &e)
	{
		ErrorHandler(e,m_ErrStr);
		//m_Rec=NULL;
		return -2;
	}
	sprintf(m_ErrStr,"Success");
	return hr;
}

HRESULT Table::MoveFirst()
{
	HRESULT hr;
	try
	{
		hr=m_Rec->MoveFirst();
	}
	catch(_com_error &e)
	{
		ErrorHandler(e,m_ErrStr);
		//m_Rec=NULL;
		return -2;
	}
	sprintf(m_ErrStr,"Success");
	return hr;
}

HRESULT Table::MoveLast()
{
	HRESULT hr;
	try
	{
		hr=m_Rec->MoveLast();
	}
	catch(_com_error &e)
	{
		ErrorHandler(e,m_ErrStr);
		//m_Rec=NULL;
		return -2;
	}
	sprintf(m_ErrStr,"Success");
	return hr;
}