#include "clock.h"

extern FNT F;
extern Display4LED2 D;
// extern sSettings Settings;
extern uint8_t Year, Month, Day, DayofWeek, Hour, Minute, Second;
/*
class Clockwork
{
private:
uint8_t _D[4];
*/
void _incSM(uint8_t p, uint8_t x){
  D._up(p+1,F.d[x%10],F.d[(x+1)%10]);
  if(x%10==9){
    D._up(p,F.d[x/10],F.d[((x+1)/10)%6]);
  }else{
    D._hold(p,F.d[x/10]);
  }
}
void _incHH(uint8_t p, uint8_t x){
// if(__AMPM); 12 am/pm
  switch(x){
    case 23:
// D._up(p,F.d[2],(__0H ? '0':F.blank));
    D._up(p  ,F.d[2],F.blank);
    D._up(p+1,F.d[3],F.d[0]);
    return;
    case 19:
    D._up(p,F.d[1],F.d[2]);
    case 9:
// D._up(p,(__0H ? '0':F.blank,F.d[1]));
    D._up(p,F.blank,F.d[1]);
    break;
    default:
    if(x<10)
// D._up(p,(__0H ? '0':F.blank,F.d[1]));
      D._hold(p,F.blank);
// D._hold(p,F.d[x/10]);
    else
      D._hold(p,F.d[x/10]);
    break;
  }
  D._up(p+1,F.d[x%10],F.d[(x+1)%10]);
}
void _SM(uint8_t p, uint8_t x){
  D._hold(p,   F.d[x/10]);
  D._hold(p+1, F.d[x%10]);
}
// void _HHMM(){
// _D[0]=Hour/10;
// _D[1]=Hour%10;
// _D[3]=Minute/10;
// _D[4]=Minute%10;
// D.B(_D);
// }
// public:
// state = NULL;
// void attachDisplay(callbackFunction newFunction) {
//   _display = newFunction;
// }
// Display D;
void Clockwork::init(){
  D.init();
  // D.setRefresh(Clockwork::HHMM);
};
// };
// void Clockwork::set(callbackFunction f){
//   D.setRefresh(f);
// };
void Clockwork::MMSS(){
  _incSM(2, Second);
  if(Second==59){
    _incSM(0, Minute);
  }else{
    _SM(0, Minute);
  }
  D.blink(2, F.dot);
};
void Clockwork::HHMM(){
  if(Minute<59){
    D._hold(0, F.d[Hour/10]);
    D._hold(1, F.d[Hour%10]);
  }
  if(Second<59){
    D._hold(2, F.d[Minute/10]);
    D._hold(3, F.d[Minute%10]);
  }else{
    _incSM(2, Minute);
    if(Minute==59){
      _incHH(0,Hour);
    }
  }
};
void Clockwork::_WD(uint8_t p, uint8_t wd){
  const uint8_t rus[7][2]={{1,5}, {3,2}, {1,6}, {5,4}, {7,6}, {3,6}, {5,0}, };
  D._hold(p,   F.rusWeekDays[rus[wd][0]]);
  D._hold(p+1, F.rusWeekDays[rus[wd][1]]);
};
void Clockwork::DDWD(){
  D._DD(0,Day);
  _WD(2,DayofWeek);
  D.on(1, F.dot);
};
void Clockwork::DDMM(){
  D._hold(0, F.d[Day/10]);
  D._hold(1, F.d[Day%10]);
  D._hold(2, F.d[(Month+1)/10]);
  D._hold(3, F.d[(Month+1)%10]);
  D.on(1, F.dot);
};
void Clockwork::Week(){
  uint8_t d[4];
  d[0]= (__A|__G|__D);
  d[1]= (__A|__G);
  d[2]= (__G|__D);
  d[3]= F.week;
  D.hold(d);
  switch(DayofWeek){
    case 1: D.blink(0,__A); break;
    case 2: D.blink(0,__G); break;
    case 3: D.blink(0,__D); break;
    case 4: D.blink(1,__A); break;
    case 5: D.blink(1,__G); break;
    case 6: D.blink(2,__G); break;
    case 0: D.blink(2,__D); break;
    default: D.blink2(3,F.minus);
  };
};
void Clockwork::YYYY(){
  D._DD(0, (Year+1900)/100);
  D._DD(2, Year%100);
  D.on(3, F.dot);
};
void Clockwork::Sunrise(){
  D._ab(  0, F.Sun.rise);
  D._hold(1, F.blank);
  D._hold(2, F.blank);
  D._hold(3, F.blank);
};
void Clockwork::Sunset(){
  D._ab(  0, F.Sun.set);
  D._hold(1, F.blank);
  D._hold(2, F.blank);
  D._hold(3, F.blank);
};
// };
