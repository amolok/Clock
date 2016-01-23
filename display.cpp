extern "C" {
  typedef void (*callbackFunction)(void);
}
/*
    A A'
    B B'
*/
class Display
{
private:
byte _AB[2][4][4]; // [frame][position]
byte f; // frame 4 frames/sec
byte b; // buffer 0: program, 1: preview // 0: showing, 1: drawing // 0: update & refresh, 1: switching
callbackFunction _updateFunc;  // 1/4 s
callbackFunction _refreshFunc; // 1 s
byte _scrollerCount;
byte _scrollerFX;
callbackFunction _next;
std::vector<char> _str;

byte _shift_U(byte X){// C->B, E->F, G->A, D->G
  return (X & __C)<<1 | (X & __E)>>1 | (X & __G)<<6 | (X & __D)>>3 ; }
byte _shift_D(byte X){// B->C, F->E, A->G, G->D
  return (X & __B)>>1 | (X & __F)<<1 | (X & __A)>>6 | (X & __G)<<3 ; }
byte _shift_R(byte X){// E->C, F->B
  return (X & __E)<<2 | (X & __F)<<4 ; }
byte _shift_L(byte X){// C->E, B->F
  return (X & __C)>>2 | (X & __B)>>4 ; }

public:
// byte brightness;
void setRefresh(callbackFunction newFunction) {
  _refreshFunc = newFunction;
};
void init(){
  _refreshFunc = NULL;
  _updateFunc = animator;
  // _saveFunc = NULL;
  f=0; // frame counter
  b=0; // display buffer number, allways 0 for update & refresh, 1 for nextState
  _scrollerCount = 0;
};

void switchBuffer(){
  b=(b++)%2;
};
void drawToBuffer(callbackFunction f){
  b=1;
  f();
  b=0;
};
/* 
  Transition effects 

  Animation: AB: f=[0-4]
  Transition: PV: PROG VIEW 

*/
void transition(callbackFunction fxFunction){
  if(fxFunction)
    fxFunction(_AB[0][3],_AB[1][3]);
  else
    VPcut();
};
void VPcut(){ // copy V to P
  for(byte i=0;i<4;i++){
    _AB[0][i][0]=_AB[1][i][0];
    _AB[0][i][1]=_AB[1][i][1];
    _AB[0][i][2]=_AB[1][i][2];
    _AB[0][i][3]=_AB[1][i][3];
  }
};
// [!   ] one position animation
void _p(byte f, byte D[4]){ // put display to frame
  _AB[b][f][0] = D[0];
  _AB[b][f][1] = D[1];
  _AB[b][f][2] = D[2];
  _AB[b][f][3] = D[3];
};
void _ab(byte p, byte AB[4]){ // put animation to frames
  _AB[b][0][p]= AB[0];
  _AB[b][1][p]= AB[1];
  _AB[b][2][p]= AB[2];
  _AB[b][3][p]= AB[3];
};
// one position fx:
void _up(byte p, byte A, byte B){
  _AB[b][0][p]= A;
  _AB[b][1][p] = _shift_U(A) | (B & __A)>>3 ;
  _AB[b][2][p] = _shift_U(_AB[b][1][p]) | (B & __B)>>1 | (B & __F)<<1 | (B & __G)<<3;
  _AB[b][3][p]= B;
};
void _down(byte p, byte A, byte B){
  _AB[b][0][p]=A;
  _AB[b][1][p] = _shift_D(A) | (B & __D)<<3 ;
  // + BC->B, BE->F, BG->A
  _AB[b][2][p] = _shift_D(_AB[b][1][p]) | (B & __C)<<1 | (B & __E)>>1 | (B & __G)<<6;
  _AB[b][3][p]=B;  
};
void _hold(byte p, byte A){
  _AB[b][0][p]= A;
  _AB[b][1][p]= A;
  _AB[b][2][p]= A;
  _AB[b][3][p]= A;
};
// [!!!!] whole display
void A(byte A[4]){ // put A (frame 0)
  _p(0, A);
};
void B(byte B[4]){ // put B (frame 3)
  _p(3, B);
};
// [0123] show some digits
void _D(byte p, byte x){ // [  1 ] digit at position
  _hold(p,   F.d[x%10]);
};
void _DD(byte p, byte x){ // [ 12 ] [02  ] 2 digits at position
  _hold(p,   F.d[(x/10)%10]);
  _hold(p+1, F.d[x%10]);
};
void _DDD(byte p, byte x){ // [123 ] [001 ] 3 digits at position
    // if(p>1){ blink2(p, __A|__G|__D); return; };
  _hold(p,   F.d[(x/100)%10]);
  _hold(p+1, F.d[(x/10)%10]);
  _hold(p+2, F.d[x%10]);
};
void _DDDD(byte x){ // [1234] [0001] 4 digits
  _hold(0, F.d[(x/1000)%10]);
  _hold(1, F.d[(x/100)%10]);
  _hold(2, F.d[(x/10)%10]);
  _hold(3, F.d[x%10]);
};

// Animation
/*
char _digits(int x){
  if((x>9999)||(x<-999))return false;
  char d[5];    
  sprintf(d, 5, "%d", x);
  return d;
};
void _incD(byte p, int x){
    // D._up(p+1,F.d[x%10],F.d[(x+1)%10]);
    // if(x%10==9){
    //   D._up(p,F.d[x/10],F.d[((x+1)/10)%6]);
    // }else{
    //   D._hold(p,F.d[x/10]);
    // }
};
void _changeD(byte p, x1, x2){
  if(x2>x1) up(p, _digits(x1),_digits(x2))
    else if (x1>x2) down(p, _digits(x1),_digits(x2));
  else _hold(p, x1);
};
*/

/* fx: blink, flash, on */

void blink(byte p, byte C){
  _AB[b][0]=_AB[b][0] | C;
  _AB[b][1]=_AB[b][1] | C;
  _AB[b][2]=_AB[b][2] & ~C;
  _AB[b][3]=_AB[b][3] & ~C;
};
void blink2(byte p, byte C){
  _AB[b][0]=_AB[b][0] | C;
  _AB[b][1]=_AB[b][1] & ~C;
  _AB[b][2]=_AB[b][2] | C;
  _AB[b][3]=_AB[b][3] & ~C;
};
void on(byte p, byte C){
  _AB[b][0]=_AB[b][0] | C;
  _AB[b][1]=_AB[b][1] | C;
  _AB[b][2]=_AB[b][2] | C;
  _AB[b][3]=_AB[b][3] | C;
};

//
// FullScreen Animation:
//
void up(byte A[4], byte B[4]){
    // ABCD
    // EFGH↑
  for(byte i=0;i<4;i++)
    _up(i, A[i], B[i]);
};
void down(byte A[4], byte B[4]){
  for(byte i=0;i<4;i++)
    _down(i, A[i], B[i]);
};
void scrollRight(byte A[4], byte B[4]){
// ABCD→EFGH BCDE CDEF DEFG EFGH
  // A[4]B[4]
  _AB[b][0][0]=A[1];
  _AB[b][0][1]=A[2];
  _AB[b][0][2]=A[3];
  _AB[b][0][3]=B[0];
  _AB[b][1][0]=A[2];
  _AB[b][1][1]=A[3];
  _AB[b][1][2]=B[0];
  _AB[b][1][3]=B[1];
  _AB[b][2][0]=A[3];
  _AB[b][2][1]=B[0];
  _AB[b][2][2]=B[1];
  _AB[b][2][3]=B[2];
  _AB[b][3][0]=B[0];
  _AB[b][3][1]=B[1];
  _AB[b][3][2]=B[2];
  _AB[b][3][3]=B[3];
};
void right(byte A[4], byte B[4]){
// ABCD→EFGH ABCD CD__ __EF EFGH
  A(A);
  _AB[b][1][0]=A[2];
  _AB[b][1][1]=A[3];
  _AB[b][1][2]=FNT.blank;
  // _AB[b][1][2]=FNT.trpile;
  _AB[b][1][3]=FNT.blank;
  _AB[b][2][0]=FNT.blank;
  // _AB[b][2][0]=FNT.trpile;
  _AB[b][2][1]=FNT.blank;
  _AB[b][2][2]=B[0];
  _AB[b][2][3]=B[1];
  B(B);
};
void left(byte A[4], byte B[4]){
// ABCD←EFGH ABCD __AB GH__ EFGH
  A(A);
  _AB[b][1][0]=FNT.blank;
  _AB[b][1][1]=FNT.blank;
  _AB[b][1][2]=A[0];
  _AB[b][1][3]=A[1];
  _AB[b][2][0]=B[2];
  _AB[b][2][1]=B[3];
  _AB[b][2][2]=FNT.blank;
  _AB[b][2][3]=FNT.blank;
  B(B);
};
void hold(byte D[4]){
  // [ABCD]
  _hold(0,D[0]);
  _hold(1,D[1]);
  _hold(2,D[2]);
  _hold(3,D[3]);
};
//
// Text writing
//
char DChar(char c){ // return char ledmap from char code
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
void text(char D[4]){ // hold [text] for four frames
  for (int i = 0; i < 4; ++i)
    D[i]=DChar(D[i]);
  hold(D);
};
//
// 1/4 s update functions:
//
void update(){// 1/4 s
  if(_updateFunc)_updateFunc();
};
// display animation
void animator(){// 1/4 s
  display(_f);
  if(_f++>3){// 1s
    if(_refreshFunc){
      var byte tb=b; b=0; // all Refresh Function will write to VIEW
      _refreshFunc(); b=tb;
    }
    _f=0;
  }
};
// display one frame -- update function -- PROG
void display(byte f){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, ~_AB[0][f][3]);
  shiftOut(dataPin, clockPin, LSBFIRST, ~_AB[0][f][2]);
  shiftOut(dataPin, clockPin, LSBFIRST, ~_AB[0][f][1]);
  shiftOut(dataPin, clockPin, LSBFIRST, ~_AB[0][f][0]);
  digitalWrite(latchPin, HIGH);
};
// direct display writing D
void print(byte D[4]){
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, ~D[0]);
  shiftOut(dataPin, clockPin, MSBFIRST, ~D[1]);
  shiftOut(dataPin, clockPin, MSBFIRST, ~D[2]);
  shiftOut(dataPin, clockPin, MSBFIRST, ~D[3]);
  digitalWrite(latchPin, HIGH);  
};
//
// String Scrolling
//
#define fx_loop     2 // [   a]→[abcd]→[fghi]→[i   ] [    ]→[   a]
#define fx_short    1 // [abcd]efghi [fghi] [abcd]
#define fx_long     3 // [abcd]→[fghi]→[i   ]→[    ] [abcd]
#define fx_longback    4 // [abcd]→[i   ]→[   a]→[abcd]
#define fx_pingpong 5 // [abcd]→[bcde]→[fghi]←[efgh]←[abcd]
#define fx_pongping 6 // ...           [fghi]←[efgh]←[abcd]
#define fx_longpingpong 7 // [   a]→[abcd]→[fghi]←[i   ]
// scroller 1/4 updater
void scroller(){// 1/4 s
  byte s[5] = _str.substr(_scrollerCount,4);
  byte D[4];
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
      byte s[5]=_str.substr(0,4);
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

};
