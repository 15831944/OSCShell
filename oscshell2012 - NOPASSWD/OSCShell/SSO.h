// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� SSO_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// SSO_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef SSO_EXPORTS
#define SSO_API __declspec(dllexport)
#else
#define SSO_API __declspec(dllimport)
#endif


SSO_API int fnSSO(void);
SSO_API int fnSSOApplication(char * type,char * apppath,char * ip, char * port, char * username, char *password, char *param);
SSO_API int fnSSOApplicationTest(char * type,char * apppath,char * ip, char * port, char * username, char *password, char *param, char * testIni);
