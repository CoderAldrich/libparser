//���ã��Ѿ�̬��lib�ļ��еĺ�����Ϣ(����������������)��ȡ����������֯�ɺ������ļ�(.flb)
//�������ļ���ʽ��ǩ��-����ͷ��-�������ƶ�-�������ݶ�


//��׼����Microsoft ����ֲ��ִ���ļ���ͨ��Ŀ���ļ���ʽ�ļ��淶������ơ�PE COFF�ļ���ʽ��

//ע�⣺����ָ�ĵ�LIB�Ǿ�̬�⣬Ҫ�ͱ�дDLL�����ɵ�lib������
//��PE COFF�ļ���ʽ���гƾ�̬���ʽΪ:�������⣩�ļ���ʽ����WinNT.h�г� Archive format.
//��PE COFF�ļ���ʽ���г���һ��LibΪ��������ʽ-��������һ��ӳ�񵼳�������ӳ��ʹ�õķ��ŵĿ�
//����ͳһ���ա�PE COFF�ļ���ʽ���Ľз����Ƶ������⣩�ļ���ʽ

//���ߣ�leeeryan	leeeryan@gmail.com


#include "LibParser.h"
#include "ObjParser.h"

CLibParser::CLibParser(void):
m_pLibImage(NULL),
m_pFlibFile(NULL),m_pNameFile(NULL),m_pDataFile(NULL)
{
}

CLibParser::~CLibParser(void)
{
	if(m_pLibImage)delete[] m_pLibImage;
}

BOOL CLibParser::Parse(PCSTR szLib)
{
	// ��ȡlib���ж��ļ��Ϸ���
	if(!LoadLib(szLib))
		return FALSE;
	//��õ�һ��Obj��Ա
	PBYTE pObjSect=GetFirstObjSection();
	if(!pObjSect)
	{
		MessageBox(NULL,"This Lib is error!","Error",MB_ICONWARNING);
		return FALSE;
	}
	//��ʼ������ļ�
	if(!InitOutPutFile(szLib))
		return FALSE;
	//��������Ŀ���ļ�(Obj)��Ա
	if(!ParseObjs(pObjSect))
		return FALSE;
	//����nam,dat�ļ�Ϊflib�ļ�
	LinkFile();

	return TRUE;
}
BOOL CLibParser::ParseObjs(PBYTE pObjSect)
{
	do 
	{
		PIMAGE_ARCHIVE_MEMBER_HEADER pAME=(PIMAGE_ARCHIVE_MEMBER_HEADER)pObjSect;
		pObjSect+=sizeof(IMAGE_ARCHIVE_MEMBER_HEADER);//ȥ��ͷ����ʣ�µľ���Obj(COFF��ʽ)

		//�ж��Ƿ��ǵ�����ʽ���Է�ֹ����İѵ����lib������̬��lib��������ֱ�ӹҵ�
		if(bImportlibraryFormat(pObjSect))
		{
			MessageBox(NULL,"This is not a Archive Format File,it's a Import Format File!",
				"WARNING",MB_ICONWARNING);
			return FALSE;
		}

		//����Ŀ���Ա(OBJ)
		CObjParser objParser;
		objParser.Parse(pObjSect,m_pNameFile,m_pDataFile,&m_FuncTable);

		//ע�⣺BYTE Size[10];Ҫ��atol((LPSTR)..)���ַ������ܵõ���ȷsize
		pObjSect += atol((LPSTR)pAME->Size) ;

		//ע�⣺������Ա֮���п�������\n����,��PE COFF �ļ���ʽ���в�û���ᵽ
		if(*pObjSect=='\n') 
			pObjSect++;

	} while (pObjSect<m_pLibImage+m_fsize);

	return TRUE;
}
//����nam,dat�ļ�Ϊflib�ļ�
//�ֱ��nam�ļ�(�������ƶ�)��dat�ļ�(�������ݶ�)���ݿ���������buf��
//�ٰ���:ǩ��-����ͷ��-�������ƶ�-�������ݶ� ˳��д��flib�ļ���
void CLibParser::LinkFile()
{
	//���������ͷ�����ƶΣ����ݶεĴ�С
	DWORD sizeofFuncHeader=(m_FuncTable.size()+1)*sizeof(FlibFuncHeader);
	DWORD sizeofNamSection=_filelength(_fileno(m_pNameFile));
	DWORD sizeofDatSection=_filelength(_fileno(m_pDataFile));
	//���������ƫ�ƣ�������������ƫ��
	DWORD baseNameOff=IMAGE_FLIB_START_SIZE+sizeofFuncHeader;
	DWORD baseDataOff=baseNameOff+sizeofNamSection;
	//��nam�ļ����ݿ�����buf��
	fclose(m_pNameFile);
	fopen_S(&m_pNameFile,m_NameFileName,"rb");
	PBYTE pNamSection=(PBYTE)malloc(sizeofNamSection);
	fread(pNamSection,sizeofNamSection,1,m_pNameFile);
	//��dat�ļ����ݿ�����buf��
	fclose(m_pDataFile);
	fopen_S(&m_pDataFile,m_DataFileName,"rb");
	PBYTE pDatSection=(PBYTE)malloc(sizeofDatSection);
	fread(pDatSection,sizeofDatSection,1,m_pDataFile);

	FlibFuncHeader funcHeader;
	//Ϊflib�ļ�д��ǩ��
	fwrite(&IMAGE_FLIB_START,IMAGE_FLIB_START_SIZE,1,m_pFlibFile);
	fflush(m_pFlibFile);
	//д�뺯��ͷ��
	FuncHeaderTable::const_iterator it=m_FuncTable.begin();
	for (;it!=m_FuncTable.end();++it)
	{
		memset(&funcHeader,0,sizeof(funcHeader));
		//�޶�ƫ��
		funcHeader.NameOff=(*it).NameOff+baseNameOff;
		funcHeader.DataOff=(*it).DataOff+baseDataOff;
		funcHeader.DataSize=(*it).DataSize;
		//д�뺯��ͷ��Ա
		fwrite(&funcHeader,sizeof(funcHeader),1,m_pFlibFile);
		fflush(m_pFlibFile);
	}
	//��һ����ȫΪ�յĺ���ͷ��Ϊ����ͷ��Ľ�β
	memset(&funcHeader,0,sizeof(funcHeader));
	fwrite(&funcHeader,sizeof(funcHeader),1,m_pFlibFile);
	fflush(m_pFlibFile);
	//д�뺯�����ƶ�
	fwrite(pNamSection,sizeofNamSection,1,m_pFlibFile);
	fflush(m_pFlibFile);
	//д�뺯�����ݶ�
	fwrite(pDatSection,sizeofDatSection,1,m_pFlibFile);
	fflush(m_pFlibFile);
	//�ͷ�֮ǰ���������buf
	free(pNamSection);
	free(pDatSection);

	//�ر��ļ�
	fclose(m_pFlibFile);
	fclose(m_pNameFile);
	fclose(m_pDataFile);
	//ɾ��nam,dat�ļ�
	remove(m_NameFileName);
	remove(m_DataFileName);
}
BOOL CLibParser::LoadLib(PCSTR szLib)
{
	FILE * pFile ;
	if(!fopen_S(&pFile,szLib,"rb"))
		return FALSE;
	m_fsize = _filelength(_fileno(pFile));

	m_pLibImage = new BYTE[m_fsize];
	if (m_pLibImage == NULL)
	{
		MessageBox(NULL,"Can't Allocate For Lib!","Error",MB_ICONWARNING);
		fclose(pFile);
		return FALSE;
	}
	fread(m_pLibImage,m_fsize,1,pFile);
	//���ǩ��,�ж��Ƿ�Ϊlib�ļ�
	if(memcmp(m_pLibImage,IMAGE_ARCHIVE_START,IMAGE_ARCHIVE_START_SIZE)!=0)
	{
		MessageBox(NULL,"This is not a Lib!","Error",MB_ICONWARNING);
		fclose(pFile);
		return FALSE;
	}

	fclose(pFile);
	return TRUE;
}
BOOL CLibParser::InitOutPutFile(PCSTR szLib)
{
	int strSize=strlen(szLib)+1;
	if (strSize>MAX_PATH)
	{
		MessageBox(NULL,"File name is too long !","Error",MB_ICONWARNING);
		return FALSE;
	}
	
	strcpy_s(m_FlibFileName,strSize,szLib);
	ModifySuffix(m_FlibFileName,".flb");
	if(!fopen_S(&m_pFlibFile,m_FlibFileName,"wb"))
		return FALSE;
	strcpy_s(m_NameFileName,strSize,szLib);
	ModifySuffix(m_NameFileName,".nam");
	if(!fopen_S(&m_pNameFile,m_NameFileName,"wb"))
		return FALSE;
	strcpy_s(m_DataFileName,strSize,szLib);
	ModifySuffix(m_DataFileName,".dat");
	if(!fopen_S(&m_pDataFile,m_DataFileName,"wb"))
		return FALSE;

	return TRUE;
}
PBYTE CLibParser::GetFirstObjSection()
{
	int iCtrl=0;
	//��һ����������Ա
	PBYTE pSect = m_pLibImage+IMAGE_ARCHIVE_START_SIZE;
	if(!pSect)return NULL;
	while(pSect)
	{
		//�ڶ�����������Ա
		if(memcmp(((PIMAGE_ARCHIVE_MEMBER_HEADER)pSect)->Name,IMAGE_ARCHIVE_LINKER_MEMBER,16)==0)
		{
			//Nothing
		}
		//�����������Ƴ�Ա
		else if(memcmp(((PIMAGE_ARCHIVE_MEMBER_HEADER)pSect)->Name,IMAGE_ARCHIVE_LONGNAMES_MEMBER,16)==0)//LONG Name
		{
			//Nothing
			//���ܳ����Ƴ�Ա��ͷ��������ڣ���������ȴ����Ϊ�ա�
		}	
		else //First Obj Section
		{
			return pSect;
		}
		//ע��BYTE Size[10];Ҫ��atol((LPSTR)..)���ַ������ܵõ���ȷsize
		PIMAGE_ARCHIVE_MEMBER_HEADER pAME=(PIMAGE_ARCHIVE_MEMBER_HEADER)pSect;
		pSect += atol((LPSTR)pAME->Size) + sizeof(IMAGE_ARCHIVE_MEMBER_HEADER);
		//������Ա֮���п�������\n����
		if(*pSect=='\n') pSect++;

		iCtrl++;//��ֹ���������Lib�ļ�����������ѭ��
		if (iCtrl>3)
		{
			break;
		}
	}
	return NULL;
}
//������ʽ�뵵��(��)��ʽ�ǳ����ƣ����ǵĲ�ͬ�㣺
//1.������ʽ����û�г���Ա
//2.������ʽ���е�Ŀ���Ա��α���Ŀ���ļ�������������Ŀ���ļ�����������COFF��ʽ
//����һ�ֳ�֮Ϊ���̸�ʽ�ĵ�����ʽ��������μ���PE COFF�ļ���ʽ����8��
BOOL CLibParser::bImportlibraryFormat(PBYTE pSect)
{
	//ͨ���ж����Ƿ��ж̸�ʽ��Ա���ж����Ƿ��ǵ�����ʽ
	WORD Sig1=*(PWORD)(pSect);
	WORD Sig2=*(PWORD)(pSect+2);
	if (Sig1==IMAGE_FILE_MACHINE_UNKNOWN&&Sig2==0xffff)
	{
		return TRUE;
	} 
	else
	{
		return FALSE;
	}
}
BOOL CLibParser::fopen_S(FILE ** _File, PCSTR _Filename,PCSTR _Mode)
{
	fopen_s(_File,_Filename,_Mode);
	if(*_File==NULL)
	{
		CHAR szError[MAX_PATH];
		sprintf_s(szError,MAX_PATH,"Can't Open %s",_Filename);
		MessageBox(NULL,szError,"Error",MB_ICONWARNING);
		return FALSE;
	}
	return TRUE;
}
//�޸ĺ�׺�����ضϳ���ԭʼ��׺���Ȳ���
void CLibParser::ModifySuffix(PCHAR filename,PCHAR pSuffix)
{
	PCHAR pDest=strrchr(filename,'.');
	do 
	{
		*pDest++=*pSuffix++;
	} while (*pDest&&*pSuffix);
	*pDest=0;
}