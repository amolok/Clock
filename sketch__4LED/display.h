#ifndef __display__
#define __display__
#include <TM1637Display.h>
#include <vector>
#include <string>
#include "settings.h"

extern "C" {
  typedef void (*callbackFunction)(void);
}
enum transition_fx
{
  fxNone, fxCut, fxUp, fxDown, fxLeft, fxRight, fxFadeLeft, fxFadeRight, fxFade
};
class Display4LED2
{
private:
uint8_t _AB[2][4][4]; // [buffer][frame][position]
uint8_t _f; // frame 4 frames/sec
uint8_t _b; // buffer 0: program, 1: preview // 0: showing, 1: drawing // 0: update & refresh, 1: switching
uint8_t _brightness; //
callbackFunction _updateFunc;  // 1/4 s
callbackFunction _refreshFunc; // 1 s
uint8_t _scrollerCount;
uint8_t _scrollerFX;
callbackFunction _next;
std::vector<char> _str;

/*
protected:
uint8_t _shift_U(uint8_t X); // C->B, E->F, G->A, D->G
uint8_t _shift_D(uint8_t X); // B->C, F->E, A->G, G->D
uint8_t _shift_R(uint8_t X); // E->C, F->B
uint8_t _shift_L(uint8_t X); // C->E, B->F
*/

public:
void init();
void setSegments(const uint8_t segments[4]);
void showNumberDec(int num, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);
void setBrightness(uint8_t b);
void setRefresh(callbackFunction newFunction);
/* 
  Transition effects 

  Animation: AB: f=[0-4]
  Transition: PV: PROG VIEW 

*/

void transition(transition_fx x);
void _cut(); // copy V to P
// [!   ] one position animation
void _p(uint8_t f, uint8_t D[4]); // put display to frame
void _ab(uint8_t p, const uint8_t AB[4]); // put animation to frames
// one position fx:
void _up(uint8_t p, uint8_t A, uint8_t B);
void _down(uint8_t p, uint8_t A, uint8_t B);
void _fadeOut(uint8_t p, const uint8_t D); 
void _hold(uint8_t p, uint8_t A);
// [!!!!] whole display
void A(uint8_t A[4]); // put A (frame 0)
void B(uint8_t B[4]); // put B (frame 3)
// [0123] show some digits
void _D(uint8_t p, uint8_t x); // [  1 ] digit at position
void _DD(uint8_t p, uint8_t x); // [ 12 ] [02  ] 2 digits at position
void _DDD(uint8_t p, uint16_t x); // [123 ] [001 ] 3 digits at position
void _DDDD(uint16_t x); // [1234] [0001] 4 digits

// Animation

/* fx: blink, flash, on */
void blink(uint8_t p, uint8_t C);
void blink2(uint8_t p, uint8_t C);
void on(uint8_t p, uint8_t C);

// FullScreen Animation:
// A[4]B[4]
// ABCD
// EFGH↑
void up(uint8_t A[4], uint8_t B[4]);
void down(uint8_t A[4], uint8_t B[4]);
// ABCD←EFGH _ABC H_ABC GH_A FGH_
void scrollLeft(uint8_t A[4], uint8_t B[4]);
void fadeLeft(uint8_t A[4], uint8_t B[4]);
// ABCD→EFGH BCDE CDEF DEFG EFGH
void scrollRight(uint8_t A[4], uint8_t B[4]);
void fadeRight(uint8_t A[4], uint8_t B[4]);
// ABCD→EFGH ABCD CD__ __EF EFGH
void right(uint8_t A[4], uint8_t B[4]);
// ABCD←EFGH ABCD __AB GH__ EFGH
void left(uint8_t A[4], uint8_t B[4]);
// [----]
void fadeOut(const uint8_t D[4]);
// [ABCD]
void hold(uint8_t D[4]);

// Text writing
// return LED-map from character code
char DChar(char c);
// hold [text] for four frames
void text(char D[4]);

// 1/4 s update functions:
void update();
// display animation
// void animator();
// display one frame -- update function -- PROG
void display(uint8_t f);
// direct display writing D
void print(uint8_t D[4]);
void drawToBuffer();
// String Scrolling
#define fx_loop     2 // [   a]→[abcd]→[fghi]→[i   ] [    ]→[   a]
#define fx_short    1 // [abcd]efghi [fghi] [abcd]
#define fx_long     3 // [abcd]→[fghi]→[i   ]→[    ] [abcd]
#define fx_longback 4 // [abcd]→[i   ]→[   a]→[abcd]
#define fx_pingpong 5 // [abcd]→[bcde]→[fghi]←[efgh]←[abcd]
#define fx_pongping 6 // ...           [fghi]←[efgh]←[abcd]
#define fx_longpingpong 7 // [   a]→[abcd]→[fghi]←[i   ]
// scroller 1/4 updater
// void scroller();
// scroll string with fx, then call next, flag "ready" when have whole string displayed
// void scroll(string s, callbackFunction next, char fx);
void debug_print();
protected:
void animator();
};
#endif
