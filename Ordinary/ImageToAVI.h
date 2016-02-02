#pragma once
#include <vfw.h> 
#pragma comment(lib,"Vfw32.lib")
 
class ImageToAVI : public base::_class
{   
protected:   
    base::Twain     _size;      //avi size
    PAVIFILE        _file;      
    PAVISTREAM      _aviStream,   
                    _aviCompressed;   
    int             _frames,_delay;   
    bool            _ok;        //state is ok   
    AVICOMPRESSOPTIONS* _aopts[1],_opts;
 
    void _initMember();
    bool _initStream(LPBITMAPINFOHEADER pbih,HWND parent);
	//关闭对象
    void Close();
	
public:   
    ImageToAVI();   
    ~ImageToAVI();   
 
    //生成的AVI文件名，delay：每幅画面停留的时间（秒），cx、cy：指定画面大小，如果为 0 ，则为第一帧图像的大小。
    bool Create(LPCTSTR fn,int delay,int cx = 0,int cy = 0);
    //添加一帧图像
    bool AddFrame(base::gdi::Bitmap& bmp,HWND parent = 0); //gdi::Bitmap，构造函数里写文件路径
    //当前状态是否可用。
    bool IsOK()
	{return _aviStream!=0;} 
};  
