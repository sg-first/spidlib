#include "stdafx.h"
#include "AVIVideo.h"

inline WORD PaletteSize(WORD color)
{
    if(color>8) return 0;
    return (1<<color)*sizeof(RGBQUAD);
}

/*int CreateDibHeader(base::Memory<byte>& data,int cx,int cy,int bits) //已经没什么用了
{
    base::gdi::Bitmap bmp;
    bmp.CreateCompatibleBitmap(cx,cy);
    if(bits==0) 
		bits = 32;
  
    DWORD hLen = sizeof(BITMAPINFOHEADER) + PaletteSize(bits);
    data.SetLength(hLen+16);
    data.Zero();
  
    LPBITMAPINFO pbi = (LPBITMAPINFO)data.Handle();
    HPALETTE hpal = (HPALETTE)::GetStockObject(DEFAULT_PALETTE);
  
    base::gdi::DC dc;
    dc.GetDC();
    hpal = dc.SelectPalette(hpal); //这种情况必须调用，或者数据格式为屏幕颜色深度格式。0是选在dc中，1是在外边
    dc.RealizePalette();
  
    pbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    //填充pbi结构，这时候不能设置色深和压缩格式，否则函数会失败
    int count = ::GetDIBits(dc,bmp,0,0,NULL,pbi,(DWORD)DIB_PAL_COLORS); //调用此函数不一定要把要提取的位图选入DC,这说明DC仅仅提供格式或功能
    if(!count) 
		goto exit;
  
    pbi->bmiHeader.biCompression = 0;
    pbi->bmiHeader.biBitCount = bits;
    //设置了压缩格式和色深后，再次填充pbi结构，目的是得到 biSizeImage 的值。
    count = ::GetDIBits(dc,bmp,0,0,NULL,pbi,(DWORD)DIB_PAL_COLORS); //调用此函数不一定要把要提取的位图选入DC,这说明DC仅仅提供格式或功能
    pbi->bmiHeader.biClrUsed = PaletteSize(bits);
	
exit:
    dc.SelectPalette(hpal);
    if(!count) return 0;
    return hLen;
}*/

ImageToAVI::ImageToAVI()
{
    AVIFileInit();
    _aopts[0] = &_opts;
    _initMember();
}

void ImageToAVI::_initMember()
{
    _size.SetValue(0,0);
    _frames = 0;
    _file = NULL;
    _aviStream = NULL;
    _aviCompressed = NULL;
    ::ZeroMemory(&_opts,sizeof(_opts));
}

bool ImageToAVI::Create(LPCTSTR fn, int delay, int cx, int cy)
{
    if(_file) return 0;
  
    base::FileSystem fs;
    fs.Initialize(fn);
    fs.Delete();
  
    HRESULT hr = AVIFileOpen(&_file,(LPWSTR)fn,OF_WRITE|OF_CREATE,NULL);
    if(hr!=AVIERR_OK) 
		return 0;
  
    _size.SetValue(cx,cy);
    _delay = delay;
  
    return 1;
}

bool ImageToAVI::_initStream(LPBITMAPINFOHEADER pbih,HWND parent)
{
    AVISTREAMINFO   strhdr;  
    ::ZeroMemory(&strhdr,sizeof(strhdr));
    strhdr.fccType      = streamtypeVIDEO;// stream type
    strhdr.fccHandler   = 0;
    strhdr.dwScale      = 1;
    strhdr.dwRate       = 1;
    strhdr.dwSuggestedBufferSize = pbih->biSizeImage;
    SetRect(&strhdr.rcFrame,0,0,pbih->biWidth,pbih->biHeight);
  
    HRESULT hr = AVIFileCreateStream(_file,&_aviStream,&strhdr);
    if(_aviStream==0){
        PrintD(L"ImageToAVI::_initStream: _aviStream = 0");
        return 0;
    }
  
    if(!AVISaveOptions(parent,0,1,&_aviStream,(LPAVICOMPRESSOPTIONS*)&_aopts)){
        PrintD(L"ImageToAVI::_initStream: AVISaveOptions failed");
        return 0;
    }
  
    hr = AVIMakeCompressedStream(&_aviCompressed,_aviStream,&_opts,NULL);
    if(_aviCompressed==0){
        base::Warning(L"Compressed");
        PrintD(L"ImageToAVI::_initStream: _aviCompressed = 0");
        return 0;
    }
  
    hr = AVIStreamSetFormat(_aviCompressed,0,pbih,pbih->biSize+PaletteSize(32)); //pbih->biClrUsed
    if(hr != AVIERR_OK){
        PrintD(L"ImageToAVI::_initStream: AVIStreamSetFormat failed"); 
        return 0;
    }
  
    return 1;
}

void ImageToAVI::Close()
{
    if(_aviStream) AVIStreamClose(_aviStream);
    if(_aviCompressed) AVIStreamClose(_aviCompressed);
    if(_file) AVIFileClose(_file);
    _initMember();
}

ImageToAVI::~ImageToAVI()
{
    Close();
    AVIFileExit();
}
  
bool ImageToAVI::AddFrame(base::gdi::Bitmap& bmp,HWND hwnd)
{
    if(!_file){
        PrintD(L"ImageToAVI::AddFrame: object not init,call Create first");
        return false;
    }
	
    if(bmp.IsNull()){
        PrintD(L"ImageToAVI::AddFrame: bmp is null");
        return 0;
    }
	
    if(_size.Value==0) _size = bmp.Size();
	
    if(bmp.Size()!=_size){
        if(!bmp.Resize(_size.x,_size.y)) return 0;
    }
	
    base::Memory<byte> dib;
    int dibLen = bmp.GetDib(dib,32);
    if(dibLen==0) 
		return false;
  
    LPBITMAPINFO pbi = (LPBITMAPINFO)dib.Handle();
    LPBITMAPINFOHEADER pbih = &pbi->bmiHeader;
  
    if(_aviCompressed==0){
        if(!_initStream(pbih,hwnd)) 
			return 0;
    }
  
    HRESULT hr = AVIStreamWrite(_aviCompressed,// stream pointer
        _frames * _delay,// time of this frame
        1,// number to write
        (LPBYTE) pbih + // pointer to data
        pbih->biSize + PaletteSize(32),
        pbih->biSizeImage,// size of this frame
        AVIIF_KEYFRAME,// flags....
        NULL,
        NULL);
		
    if(hr!=AVIERR_OK){
        Close();
        return false;
    }
	
    _frames++;
    return true;
}
