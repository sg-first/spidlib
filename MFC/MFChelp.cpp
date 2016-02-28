#include "MFChelp.h"

void MFChelp::SaveBmpTofile(HBITMAP hbmp, CString path)
{
	//定义文件头结构
	BITMAPFILEHEADER fileHead;                                              
	int fileHeadLen = sizeof( BITMAPFILEHEADER );
	//定义图像信息结构
	BITMAPINFOHEADER bmpHead;
	int bmpHeadLen =sizeof( BITMAPINFOHEADER );
	BITMAP bmpObj;
	GetObject( hbmp, sizeof(BITMAP), &bmpObj );
 
	DWORD fileSizeInByte;//文件总的字节大小
	//获取系统颜色深度,即每个像素用多少位表还示
	DWORD PixelSizeInBit;               
	CDC srcDC;//系统屏幕设备描述表
 
	srcDC.CreateDC( "DISPLAY", NULL, NULL, NULL);
 
	PixelSizeInBit=srcDC.GetDeviceCaps( BITSPIXEL ) * srcDC.GetDeviceCaps( PLANES );
	fileSizeInByte = fileHeadLen + bmpHeadLen + bmpObj.bmWidth*bmpObj.bmHeight*PixelSizeInBit/8;
	//初始化文件头结构
	fileHead.bfOffBits = fileHeadLen + bmpHeadLen;
	fileHead.bfReserved1=0;
	fileHead.bfReserved2=0;
	fileHead.bfSize = fileSizeInByte;
	fileHead.bfType = 0x4D42;
	///初始图像信息结构
	bmpHead.biBitCount = PixelSizeInBit;
	bmpHead.biCompression = BI_RGB;
	bmpHead.biPlanes = 1;
	bmpHead.biHeight = bmpObj.bmHeight;
	bmpHead.biWidth = bmpObj.bmWidth;
	bmpHead.biSize = bmpHeadLen;
	//为文件分配空间
	PBYTE pFile = new byte[ fileSizeInByte ];
	memset( pFile, 0, fileSizeInByte );
	//填充文件头部
	memcpy( pFile, (PBYTE)&fileHead, fileHeadLen);
	//填充文件信息头部结构
	memcpy( pFile+fileHeadLen, (PBYTE)&bmpHead, bmpHeadLen);
	//填充像素部分
	GetDIBits( srcDC.m_hDC, hbmp, 0, bmpObj.bmHeight, pFile+fileHeadLen+bmpHeadLen, (LPBITMAPINFO)(pFile+fileHeadLen), DIB_RGB_COLORS);
	//打开文件并写入数据
	HANDLE hFile;
	hFile=CreateFile( path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
 
	if(hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox( "创建文件失败" );
		return;
	}
 
	DWORD nByteTransfered;
	WriteFile( hFile, pFile, fileSizeInByte, &nByteTransfered, NULL);
	CloseHandle( hFile );
	//清理
	delete pFile;
	srcDC.DeleteDC();
}

CString MFChelp::BrowseFolder(CString title)
{
	UpdateData(true);
	char szDir[MAX_PATH];//用来接收文件名
	BROWSEINFO bi;
	ITEMIDLIST*pidl;
	memset(&bi,0,sizeof BROWSEINFO); 
	bi.hwndOwner=m_hWnd;
	bi.pidlRoot=NULL;
	bi.pszDisplayName=szDir;//pszDisplayName：用来保存用户选中的目录字符串的内存地址(缓冲区)
	bi.lpszTitle=title; // 这一行将显示在对话框的顶端
	//描述窗口的选项：包含状态文本、显示目录和文件、只返回文件系统目录（不好用呢）、返回系统一个节点、
	bi.ulFlags=BIF_STATUSTEXT|BIF_BROWSEINCLUDEFILES|BIF_RETURNONLYFSDIRS|BIF_RETURNFSANCESTORS ;
	bi.lpfn=NULL;//lpfn：应用程序定义的浏览对话框回调函数的地址。当对话框中的事件发生时，该对话框将调用回调函数。该参数可用为NULL。
	bi.lParam=0;//lParam：对话框传递给回调函数的一个参数指针。
	bi.iImage=0;//iImage：与选中目录相关的图像。该图像将被指定为系统图像列表中的索引值。
	pidl=SHBrowseForFolder(&bi);
	if(pidl==NULL)
	{return NULL;}
	else
	{SHGetPathFromIDList(pidl,szDir);} //显示文件路径，不然szDir 只保存一个文件名
	CString m_strFileName=szDir;
	UpdateData(false);
	return m_strFileName;
}

void MFChelp::CheckAllFile(string sPath,string name,string limit="*.*",void(*callback)(string))
{
	CFileFind ff; 
	if(sPath.Right(1) != "\\") //保证目录是以\结尾的 
	{sPath += "\\"; }
	string panding;
	panding=sPath;
	panding+=name;
	sPath += limit;
	BOOL bFound = ff.FindFile(sPath); 
	while(bFound)
	{ 
		bFound = ff.FindNextFile(); 
		string sFilePath = ff.GetFilePath();
		if (sFilePath==panding)
		{
			callback(panding);//路径传过去
		}
		sFilePath = ff.GetFilePath(); 
		if(ff.IsDirectory())//IsDirectory()为检测此路径是否指向目录
		{ 
			if(!ff.IsDots())//IsDots()检测此路径是否为缺省目录
				CheckAllFile(sFilePath,name);
		}
	} 
	ff.Close();
}

void MFChelp::FTPdownload(string address,string username,string password,int port,string FTPpath,string savePath)
{
	CInternetSession* m_pInetSession;//网络操作
	CFtpConnection* m_pFtpConnection;//FTP操作
	m_pInetSession = new CInternetSession( AfxGetAppName(), 1,PRE_CONFIG_INTERNET_ACCESS);//关于绘画的定义
	m_pFtpConnection = m_pInetSession->GetFtpConnection(address,username,password,port);//建立连接
	m_pFtpConnection->GetFile(_T(FTPpath), _T(savePath));//下载文件（注意FTP路径前面加/）
	//断开连接
	if(m_pFtpConnection != NULL)
	{
		m_pFtpConnection->Close();
		delete m_pFtpConnection;
	}
	delete m_pInetSession;
}

void MFChelp::CloseProcess(CString strProgram)
{ 
	HANDLE handle; //定义CreateToolhelp32Snapshot系统快照句柄 
	HANDLE handle1; //定义要结束进程句柄 
	handle =CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);//获得系统快照句柄 
	PROCESSENTRY32 *info; //定义PROCESSENTRY32结构字指 
	//PROCESSENTRY32 结构的 dwSize 成员设置成 sizeof(PROCESSENTRY32)
	info = new PROCESSENTRY32; 
	info->dwSize = sizeof(PROCESSENTRY32); 
	//调用一次 Process32First 函数，从快照中获取进程列表 
	Process32First(handle, info); 
	//重复调用 Process32Next，直到函数返回 FALSE 为止 
	while(Process32Next(handle, info) != FALSE) 
	{ 
		CString strTmp = info->szExeFile; //指向进程名字 
		//strcmp字符串比较函数同要结束相同 
		//if(strcmp(c, info->szExeFile) == 0 ) 
		if (strProgram.CompareNoCase(info->szExeFile) == 0 ) 
		{ 
			//PROCESS_TERMINATE表示为结束操作打开,FALSE=可继承,info->th32ProcessID=进程ID 
			handle1 = OpenProcess(PROCESS_TERMINATE, FALSE, info->th32ProcessID); 
			//结束进程 
			TerminateProcess(handle1, 0); 
		} 
	}
	delete info;
	CloseHandle(handle);
}