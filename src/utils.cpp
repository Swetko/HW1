#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "utils.h"
#include "image.h"

int tests_total = 0;
int tests_fail = 0;

int same_image(const Image& a, const Image& b) { return a==b; }

bool operator ==(const Image& a, const Image& b)
  {
  if(a.w != b.w || a.h != b.h || a.c != b.c) 
    {
    printf("Expected %d x %d x %d image, got %d x %d x %d\n", b.w, b.h, b.c, a.w, a.h, a.c);
    return 0;
    }
  
  for(int i = 0; i < a.w*a.h*a.c; ++i) if(!within_eps(a.data[i], b.data[i])) 
    {
    printf("The value at %d %d %d should be %f, but it is %f! \n", i/(a.w*a.h), (i%(a.w*a.h))/a.h, (i%(a.w*a.h))%a.h, b.data[i], a.data[i]);
    return 0;
    }
  return 1;
  }



template <size_t TSZ>
void TiledTranspose(Image& img_out, const Image& img_in, int c)
  {
  const size_t w = img_in.w;
  const size_t h = img_in.h;
  const size_t BPP = sizeof(float);
  
  float d[TSZ][TSZ];
  
  for(size_t xin = 0; xin < w; xin += TSZ)
    for(size_t yin = 0; yin < h; yin += TSZ)
      {
      const size_t xspan = min(TSZ, w - xin);
      const size_t yspan = min(TSZ, h - yin);
      const size_t dmin = min(xspan, yspan);
      const size_t dmax = max(xspan, yspan);
      const size_t xout = yin;
      const size_t yout = xin;
      
      for(size_t y = 0; y < yspan; y++)
        memcpy(d[y], &img_in(xin, yin + y, c), xspan * BPP);
      
      for(size_t x = 0; x < dmin; x++)
        for(size_t y = x + 1; y < dmax; y++)
          swap(d[x][y], d[y][x]);
      
      for(size_t y = 0; y < xspan; y++)
        memcpy(&img_out(xout, yout + y,  c), d[y], yspan * BPP);
      }
  }




Image Image::transpose(void) const
  {
  //TIME(1);
  Image ret(h,w,c);
  
  if(c>1)
    {
    vector<thread> th;
    for(int c=0;c<this->c;c++)th.push_back(thread([&ret,this,c](){TiledTranspose<80>(ret,*this,c);}));
    for(auto&e1:th)e1.join();
    }
  else TiledTranspose<80>(ret,*this,0);
  
  return ret;
  }


inline float dot_product(const float* a, const float* b, int n)
  {
  float sum=0;
  for(int q1=0;q1<n;q1++)sum+=a[q1]*b[q1];
  return sum;
  }


Image Image::abs(void) const 
  {
  Image ret=*this;
  for(int q2=0;q2<h;q2++)for(int q1=0;q1<w;q1++)
    for(int q3=0;q3<c;q3++)
      {
      float a=pixel(q1,q2,q3);
      ret(q1,q2,q3)=fabsf(a);
      }
  return ret;
  }

Image Image::rgb_to_grayscale(void) const { return ::rgb_to_grayscale(*this); }

void Image::set_channel(int ch, const Image& im)
  {
  assert(im.c==1 && ch<c && ch>=0);
  assert(im.w==w && im.h==h);
  memcpy(&pixel(0,0,ch),im.data,sizeof(float)*im.size());
  }

Image Image::get_channel(int ch) const 
  {
  assert(ch<c && ch>=0);
  Image im(w,h,1);
  memcpy(im.data,&pixel(0,0,ch),sizeof(float)*im.size());
  return im;
  }
