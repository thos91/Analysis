// system includes
#include <iostream>

// user includes
#include "wgColor.hpp"

using namespace std;

int wgColor::wgcolors[100];


wgColor::wgColor(){
  for(int i=0;i<100;i++){
    if(i<16){
      if(i%4==0)wgColor::wgcolors[i]=900-i/4;
      if(i%4==1)wgColor::wgcolors[i]=800-(i-1)/4;
      if(i%4==2)wgColor::wgcolors[i]=910-(i-2)/4;
      if(i%4==3)wgColor::wgcolors[i]=810-(i-3)/4;
    }else if(i==16){ 
      wgColor::wgcolors[i]=636;
    }else if(i>16 && i<33){
      if(i%4==0)wgColor::wgcolors[i]=860-i/4;
      if(i%4==1)wgColor::wgcolors[i]=880-(i-1)/4;
      if(i%4==2)wgColor::wgcolors[i]=870-(i-2)/4;
      if(i%4==3)wgColor::wgcolors[i]=890-(i-3)/4;
    }else if(i==33){
      wgColor::wgcolors[i]=604;
    }else if(i>33 && i<50){
      if(i%4==0)wgColor::wgcolors[i]=840-i/4;
      if(i%4==1)wgColor::wgcolors[i]=820-(i-1)/4;
      if(i%4==2)wgColor::wgcolors[i]=850-(i-2)/4;
      if(i%4==3)wgColor::wgcolors[i]=830-(i-3)/4;
    }else if(i==50){
      wgColor::wgcolors[i]=420;
    }else if(i>=60){
      if(i%6==0)wgColor::wgcolors[i]=810-i/6;
      if(i%6==1)wgColor::wgcolors[i]=860-(i-1)/6;
      if(i%6==2)wgColor::wgcolors[i]=840-(i-2)/6;
      if(i%6==3)wgColor::wgcolors[i]=900-(i-3)/6;
      if(i%6==4)wgColor::wgcolors[i]=880-(i-4)/6;
      if(i%6==5)wgColor::wgcolors[i]=820-(i-5)/6;
       
    }
  }
}
