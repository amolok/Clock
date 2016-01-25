#include "display.h"
#include "font.h"
/*
    A A'
    B B'
*/

TM1637Display display(CLK, DIO);

Display4LED2::Display4LED2()
{
  _refreshFunc = NULL;
//  _updateFunc = &Display4LED2::animator;
  _f=0; _b=0; // set frame counter and display buffer number, allways 0 for update & refresh, 1 for nextState
  _scrollerCount = 0;
  _brightness=0xff;
};
/* void Display4LED2::setBrightness(uint8_t brightness){
  display.setBrightness(brightness);
};*/


// C->B, E->F, G->A, D->G
uint8_t _shift_U(uint8_t X){
  return (X & __C)>>1 | (X & __E)<<1 | (X & __G)>>6 | (X & __D)<<3 ; }
// B->C, F->E, A->G, G->D
uint8_t _shift_D(uint8_t X){
  return (X & __B)<<1 | (X & __F)>>1 | (X & __A)<<6 | (X & __G)>>3 ; }
// E->C, F->B
uint8_t _shift_R(uint8_t X){
  return (X & __E)>>2 | (X & __F)>>4 ; }
// C->E, B->F
uint8_t _shift_L(uint8_t X){
  return (X & __C)<<2 | (X & __B)<<4 ; }

void Display4LED2::setRefresh(callbackFunction newFunction){
  _refreshFunc = newFunction;
};
void Display4LED2::drawToBuffer(){
  _b=1;
};
/* 
  Transition effects 

  Animation: AB: f=[0-4]
  Transition: PV: PROG VIEW 
*/

void Display4LED2::transition(transition_fx x){
  switch(x){
    case fxUp:
    up(_AB[0][3],_AB[1][3]);
    break;
    case fxDown:
    down(_AB[0][3],_AB[1][3]);
    break;
    case fxLeft:
    left(_AB[0][3],_AB[1][3]);
    break;
    case fxRight:
    right(_AB[0][3],_AB[1][3]);
    break;
    default:
    _cut();
  }
};
// copy V to P
void Display4LED2::_cut(){
  for(uint8_t i=0;i<4;i++){
    _AB[0][i][0]=_AB[1][i][0];
    _AB[0][i][1]=_AB[1][i][1];
    _AB[0][i][2]=_AB[1][i][2];
    _AB[0][i][3]=_AB[1][i][3];
  }
};
// [!   ] one position animation
// put display to frame
void Display4LED2::_p(uint8_t f, uint8_t D[4]){
  _AB[_b][f][0] = D[0];
  _AB[_b][f][1] = D[1];
  _AB[_b][f][2] = D[2];
  _AB[_b][f][3] = D[3];
};
// put animation to frames
void Display4LED2::_ab(uint8_t p, const uint8_t AB[4]){ 
  _AB[_b][0][p]= AB[0];
  _AB[_b][1][p]= AB[1];
  _AB[_b][2][p]= AB[2];
  _AB[_b][3][p]= AB[3];
};
// one position fx:
void Display4LED2::_up(uint8_t p, uint8_t A, uint8_t B){
  _AB[_b][0][p]= A;
  _AB[_b][1][p] = _shift_U(A) | (B & __A)<<3 ;
  _AB[_b][2][p] = _shift_U(_AB[_b][1][p]) | (B & __B)<<1 | (B & __F)>>1 | (B & __G)>>3;
  _AB[_b][3][p]= B;
};
void Display4LED2::_down(uint8_t p, uint8_t A, uint8_t B){
  _AB[_b][0][p]=A;
  _AB[_b][1][p] = _shift_D(A) | (B & __D)>>3 ;
  // + BC->B, BE->F, BG->A
  _AB[_b][2][p] = _shift_D(_AB[_b][1][p]) | (B & __C)>>1 | (B & __E)<<1 | (B & __G)>>6;
  _AB[_b][3][p]=B;  
};
void Display4LED2::_hold(uint8_t p, uint8_t A){
  _AB[_b][0][p]= A;
  _AB[_b][1][p]= A;
  _AB[_b][2][p]= A;
  _AB[_b][3][p]= A;
};
// [!!!!] whole display
void Display4LED2::A(uint8_t A[4]){ // put A (frame 0)
  _p(0, A);
};
void Display4LED2::B(uint8_t B[4]){ // put B (frame 3)
  _p(3, B);
};
// [0123] show some digits
void Display4LED2::_D(uint8_t p, uint8_t x){ // [  1 ] digit at position
  _hold(p,   F.d[x%10]);
};
void Display4LED2::_DD(uint8_t p, uint8_t x){ // [ 12 ] [02  ] 2 digits at position
  _hold(p,   F.d[(x/10)%10]);
  _hold(p+1, F.d[x%10]);
};
void Display4LED2::_DDD(uint8_t p, uint8_t x){ // [123 ] [001 ] 3 digits at position
    // if(p>1){ blink2(p, __A|__G|__D); return; };
  _hold(p,   F.d[(x/100)%10]);
  _hold(p+1, F.d[(x/10)%10]);
  _hold(p+2, F.d[x%10]);
};
void Display4LED2::_DDDD(uint8_t x){ // [1234] [0001] 4 digits
  _hold(0, F.d[(x/1000)%10]);
  _hold(1, F.d[(x/100)%10]);
  _hold(2, F.d[(x/10)%10]);
  _hold(3, F.d[x%10]);
};

// Animation

/* fx: blink, flash, on */

void Display4LED2::blink(uint8_t p, uint8_t C){
  _AB[_b][0][p]=_AB[_b][0][p] | C;
  _AB[_b][1][p]=_AB[_b][1][p] | C;
  _AB[_b][2][p]=_AB[_b][2][p] & ~C;
  _AB[_b][3][p]=_AB[_b][3][p] & ~C;
};
void Display4LED2::blink2(uint8_t p, uint8_t C){
  _AB[_b][0][p]=_AB[_b][0][p] | C;
  _AB[_b][1][p]=_AB[_b][1][p] & ~C;
  _AB[_b][2][p]=_AB[_b][2][p] | C;
  _AB[_b][3][p]=_AB[_b][3][p] & ~C;
};
void Display4LED2::on(uint8_t p, uint8_t C){
  _AB[_b][0][p]=_AB[_b][0][p] | C;
  _AB[_b][1][p]=_AB[_b][1][p] | C;
  _AB[_b][2][p]=_AB[_b][2][p] | C;
  _AB[_b][3][p]=_AB[_b][3][p] | C;
};

//
// FullScreen Animation:
//
void Display4LED2::up(uint8_t A[4], uint8_t B[4]){
    // ABCD
    // EFGH↑
  for(uint8_t i=0;i<4;i++)
    _up(i, A[i], B[i]);
};
void Display4LED2::down(uint8_t A[4], uint8_t B[4]){
  for(uint8_t i=0;i<4;i++)
    _down(i, A[i], B[i]);
};
void Display4LED2::scrollRight(uint8_t A[4], uint8_t B[4]){
// ABCD→EFGH BCDE CDEF DEFG EFGH
  // A[4]B[4]
  _AB[_b][0][0]=A[1];
  _AB[_b][0][1]=A[2];
  _AB[_b][0][2]=A[3];
  _AB[_b][0][3]=B[0];
  _AB[_b][1][0]=A[2];
  _AB[_b][1][1]=A[3];
  _AB[_b][1][2]=B[0];
  _AB[_b][1][3]=B[1];
  _AB[_b][2][0]=A[3];
  _AB[_b][2][1]=B[0];
  _AB[_b][2][2]=B[1];
  _AB[_b][2][3]=B[2];
  _AB[_b][3][0]=B[0];
  _AB[_b][3][1]=B[1];
  _AB[_b][3][2]=B[2];
  _AB[_b][3][3]=B[3];
};
void Display4LED2::right(uint8_t _A[4], uint8_t _B[4]){
// ABCD→EFGH ABCD CD__ __EF EFGH
  A(_A);
  _AB[_b][1][0]=_A[2];
  _AB[_b][1][1]=_A[3];
  _AB[_b][1][2]=F.blank;
  // _AB[_b][1][2]=F.trpile;
  _AB[_b][1][3]=F.blank;
  _AB[_b][2][0]=F.blank;
  // _AB[_b][2][0]=F.trpile;
  _AB[_b][2][1]=F.blank;
  _AB[_b][2][2]=_B[0];
  _AB[_b][2][3]=_B[1];
  B(_B);
};
void Display4LED2::left(uint8_t _A[4], uint8_t _B[4]){
// ABCD←EFGH ABCD __AB GH__ EFGH
  A(_A);
  _AB[_b][1][0]=F.blank;
  _AB[_b][1][1]=F.blank;
  _AB[_b][1][2]=_A[0];
  _AB[_b][1][3]=_A[1];
  _AB[_b][2][0]=_B[2];
  _AB[_b][2][1]=_B[3];
  _AB[_b][2][2]=F.blank;
  _AB[_b][2][3]=F.blank;
  B(_B);
};
void Display4LED2::hold(uint8_t D[4]){
  // [ABCD]
  _hold(0,D[0]);
  _hold(1,D[1]);
  _hold(2,D[2]);
  _hold(3,D[3]);
};
//
// Text writing
//
uint8_t DChar(char c){ // return LED-map from character code
  if((c>='0')||(c<='9'))
    return F.d[c-'0'];
  if((c>='A'||(c<='Z')))
    return F.AbC[c-'A'];
  if((c>='a'||(c<='z')))
    return F.AbC[c-'a'];
  switch(c){
    case 0: return F.blank;
    case ' ': return F.blank;
    case '-': return F.minus;
    case '~': return F.grad;
    case '.': return F.dot;
    default: return F.blank;
  }
};
void Display4LED2::text(char D[4]){ // hold [text] for four frames
  uint8_t T[4];
  for (int i = 0; i < 4; ++i)
    T[i]=DChar(D[i]);
  hold(T);
};
//
// 1/4 s update functions:
//
void Display4LED2::update(){// 1/4 s
//  if(_updateFunc)_updateFunc();
  Display4LED2::animator();
};

// display animation
void Display4LED2::animator(){// 1/4 s
  display(_f++);
  if(_f>3){// 1s
    if(_refreshFunc){
      uint8_t tb=_b; _b=0; // all Refresh Function will write to VIEW
      _refreshFunc();
      _b=tb;
    }
    _f=0;
  }
};
// display one frame -- update function -- PROG
void Display4LED2::display(uint8_t f){
//  _D(3,f);
//  setBrightness((uint8_t)(f<<1)+8);
  print((_AB[0][f]));
};
void Display4LED2::setSegments(const uint8_t segments[4]){
  ::display.setSegments(segments, 4, 0);
}
void Display4LED2::showNumberDec(int num, bool leading_zero, uint8_t length, uint8_t pos)
{
  ::display.showNumberDec(num, leading_zero, length, pos);
}
void Display4LED2::setBrightness(uint8_t b){
  _brightness=b;
  ::display.setBrightness(_brightness);
  };
// direct display writing D
void Display4LED2::print(uint8_t D[4]){
  setBrightness(_brightness);
  ::display.setSegments(D,4,0);
};

// String Scrolling
#define fx_loop     2 // [   a]→[abcd]→[fghi]→[i   ] [    ]→[   a]
#define fx_short    1 // [abcd]efghi [fghi] [abcd]
#define fx_long     3 // [abcd]→[fghi]→[i   ]→[    ] [abcd]
#define fx_longback    4 // [abcd]→[i   ]→[   a]→[abcd]
#define fx_pingpong 5 // [abcd]→[bcde]→[fghi]←[efgh]←[abcd]
#define fx_pongping 6 // ...           [fghi]←[efgh]←[abcd]
#define fx_longpingpong 7 // [   a]→[abcd]→[fghi]←[i   ]
// scroller 1/4 updater
/*
void scroller()
{
  uint8_t s[5] = _str.substr(_scrollerCount,4);
  uint8_t D[4];
  D[0]=DChar(s[0]);
  D[1]=DChar(s[1]);
  D[2]=DChar(s[2]);
  D[3]=DChar(s[3]);
  D.print(D);

  switch(_scrollerFX){
    case fx_loop:
    if(_scrollerCount+4>strlen(_str) _scrollerCount=0;
      break;
      case fx_short:
      case fx_long:
      case fx_longback:
      if(_scrollerCount+4>strlen(_str){
        uint8_t s[5]=_str.substr(0,4);
        D.hold(D);
        _updateFunc=animator;
      }
      break;
      case fx_pingpong:
      if(_scrollerCount+4>strlen(_str)){
        _scrollerFX=fx_pongping;
        return;
      }else _scrollerCount++;
      break;
      case fx_pongping:
      if(_scrollerCount==0){
        _scrollerFX=fx_pingpong;
        return;
      }else{
        _scrollerCount--;
      }
  }
};
// scroll string with fx, then call next, flag "ready" when have whole string displayed
void scroll(string s, callbackFunction next, char fx){
  _str.clear();
  _str.reserve(strlen(s)+8);
  strcpy(_str,s);
  _scrollerFX=fx;
  switch(fx){
    case fx_loop:
    strcat(_str,'    ');
    break;
    case fx_long:
    strcat(_str,'    ');
    _scrollerFX=fx_short;
    break;
    case fx_longback:
    strcat(_str,'   ');
    strcat(_str,_str.substr(0,4));
    _scrollerFX=fx_short;
    break;
    case fx_longpingpong:
    string s[strlen(_str)+8];
    strcpy(s,'    ');
    strcat(s,_str);
    strcat(_str,'    ');
    _scrollerFX=fx_pingpong;
    break;
    default:
    _scrollerFX=fx_loop;
  }
  ready=false;
  _updateFunc=scroller; // change UPD 1/4 s
  _next=next; // chain next
};
*/

