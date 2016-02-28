class MFChelp
{
public:
	//保存图像为文件
	static void SaveBmpTofile(HBITMAP hbmp, CString path); //需保存的图像的句柄，保存路径
	//浏览文件夹
	static CString BrowseFolder(CString title); /*返回值没有\*/
	//搜索文件
	static void CheckAllFile(string sPath,string name,string limit="*.*",void(*callback)(string)); //搜索的路径，文件名，通配符，找到后的回调
	//FTP下载
	static void FTPdownload(string address,string username,string password,int port,string FTPpath,string savePath);
	//结束进程
	static void CloseProcess(CString strProgram);
};