#include "string.h"

/*����Ŀ���ַ����ĳ��ȣ�searchΪ���滻�����ַ���
replace�滻�����ַ���,pSrcStrΪ�滻ǰ���ܵ��ַ���
pResultΪ�滻����ܵ��ַ���,����ֵΪ�滻�˶��ٸ�
search�ַ�����LenΪ��������ָ��洢��pResult����
*/
int str_replace(const char *search ,const char *replace, const char *pSrcStr,char* pResult,unsigned int Len)
{
	//ZXABC'MNT'DFGK'MNT'KLOP'MNT'YUIT  ��MNT�滻ΪBMPQ
	//ZXABC'BMPQ
	int len=0,L1,L2,search_num=0;
	int len_search=strlen(search);
	int len_replace=strlen(replace);
	len=strlen(pSrcStr);
	char* srcStr=(char*)malloc(len+len_search+1);
	char* psrcStr=srcStr;
	char* pSep=NULL;
	char* old_sub=(char*)malloc(len+1);
	memset(old_sub,0,len+1);
	memset(pResult,0,Len);
	memset(srcStr,0,len+len_search+1);
	strcpy(srcStr,pSrcStr);
	strcat(srcStr,search);//��Դ�ַ���β����Ӵ��滻�Ӵ����ɷ��ѭ��
	while ((pSep=strstr(srcStr,search))!=NULL)
	{
		++search_num;
		memset(old_sub,0,len+1);
		L1=strlen(srcStr);
		L2=strlen(pSep);
		memmove(old_sub,srcStr,L1-L2);
		len=strlen(old_sub);
		srcStr+=len+len_search;
		if (1==search_num)
		{
			strcpy(pResult,old_sub);
		}
		else
		{
			strcat(pResult,old_sub);
		}
		if (NULL==strstr(srcStr,search))
		{
			break;
		}
		strcat(pResult,replace);
	}
	free(psrcStr);
	free(old_sub);
	return search_num;
}