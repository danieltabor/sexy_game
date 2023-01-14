/*
 * Copyright (c) 2022, Daniel Tabor
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifdef __WIN32__
#include<windows.h>
#else
#define SDL_MAIN_HANDLED 1
#include<string.h>
#endif

#include<stdio.h>
#include<math.h>
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>
#include<SDL2/SDL_mixer.h>
#include"SDL2_framerate.h"
#include"resources.h"

#define PI 3.14159265358979323846

#ifdef WIN32
void fatal(char* message) {
  MessageBox(0,message,"Fatal Error",MB_OK|MB_ICONEXCLAMATION);
  SDL_Quit();
  exit(1);
}
#else
void fatal(char* message) {
  fprintf(stderr,"%s\n",message);
  SDL_Quit();
  exit(1);
}
#endif

#define MIN_ANGLE -135.0
#define MAX_ANGLE  135.0
#define MIN_LENGTH  500
#define MAX_LENGTH 2000
#define GIRL_MAX    150
#define BOOB_LAG     10
#define GIRL_COUNT    3

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Haptic* haptic = 0;
SDL_AudioDeviceID audio;

SDL_Texture* hud;
SDL_Texture* dial;
SDL_Texture* bg[GIRL_COUNT];
SDL_Texture* girl[GIRL_COUNT];
SDL_Texture* boobs[GIRL_COUNT];
Mix_Chunk* snd[GIRL_COUNT];

int active_girl = 0;
double dial_angles[3]  = { 0.0, 0.0, 0.0 };

unsigned int cycle_start=0;
int cycle_ramp_down=0;
int cycle_ramp_up=0;
int cycle_length=0;
double cycle_mag=0.0;


void init_wave() {
  SDL_Joystick* joy;
  int i;
  int hapcount = SDL_NumHaptics();
  int supported;

  for( i=0; i<hapcount && haptic==0; i++ ) {
    haptic = SDL_HapticOpen(i); 
    if( SDL_HapticRumbleInit(haptic) ) {
      SDL_HapticClose(haptic);
      haptic = 0;
    }

  }
  if( haptic == 0 ) {
    fatal("This program requires a controller with rumble capabilities");
  }
}

void generate_wave(unsigned int ticks) {
  int length_ms;
  double mag_percent;
  double ramp_percent;
  mag_percent = (dial_angles[0]-MIN_ANGLE) / (MAX_ANGLE - MIN_ANGLE);
  length_ms = MAX_LENGTH - ((dial_angles[1]-MIN_ANGLE) / (MAX_ANGLE - MIN_ANGLE) * (MAX_LENGTH-MIN_LENGTH));
  ramp_percent = (dial_angles[2]-MIN_ANGLE) / (MAX_ANGLE - MIN_ANGLE);
  if( length_ms == 0 ) { length_ms = 1; }
  if( ramp_percent >= 0.98 ) { ramp_percent = 0.98; }
  
  if( cycle_start == 0 ) {
    cycle_start = ticks;
  }
  cycle_length = length_ms;
  if( ramp_percent > 0.0 ) {
    cycle_ramp_down = length_ms - (length_ms*ramp_percent);
    cycle_ramp_up = length_ms - (length_ms*ramp_percent/2.0);
  }
  else {
    cycle_ramp_down = length_ms;
    cycle_ramp_up = length_ms;
  }
  cycle_mag = mag_percent;
}

int do_wave(unsigned int ticks) {
  double current_mag = cycle_mag;
  if( ticks > cycle_start+cycle_length ) {
    cycle_start = ticks;
  }
  else if( ticks > cycle_start+cycle_ramp_up ) {
    current_mag =  cycle_mag*(double)(ticks-cycle_start - cycle_ramp_up) / (double)(cycle_length - cycle_ramp_up);
  }
  else if( ticks > cycle_start+cycle_ramp_down ) {
    current_mag = cycle_mag - (cycle_mag*(double)(ticks-cycle_start - cycle_ramp_down) / (double)(cycle_ramp_up - cycle_ramp_down));
  }

  SDL_HapticRumblePlay(haptic,current_mag, SDL_HAPTIC_INFINITY);
}

void finish_wave() {
  if( haptic == 0 ) { 
    SDL_HapticRumbleStop(haptic);
    SDL_HapticClose(haptic);
  }
}

void init_sdl() {
  if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_JOYSTICK|SDL_INIT_HAPTIC) != 0 ) {
    fatal("Unable to initialize SDL");
  }
  if( IMG_Init(IMG_INIT_PNG) == 0 ) {
    fatal("Unable to intialize SDL_image");
  }
  if( Mix_Init(MIX_INIT_OGG) == 0 ) {
    fatal("Unable to initialze SDL_mixer");
  }
  if( Mix_OpenAudio(44100,AUDIO_S16,1,1024) ) {
    fatal("Unable to open audio");
  }
  window = SDL_CreateWindow("Sexy",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            800,600,0);
  if( window == 0 ) {
    fatal("Unable to create window");
  }
  renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
  if( renderer == 0 ) {
    fatal("Unable to create renderer");
  }
}

void load_resources() {
  if( (hud   = IMG_LoadTexture_RW(renderer,getres(HUD_PNG),1))  == 0 || 
      (dial  = IMG_LoadTexture_RW(renderer,getres(DIAL_PNG),1)) == 0 || 
      (bg[0]    = IMG_LoadTexture_RW(renderer,getres(BG0_PNG),1))   == 0 || 
      (girl[0]  = IMG_LoadTexture_RW(renderer,getres(GIRL0_PNG),1)) == 0 || 
      (boobs[0] = IMG_LoadTexture_RW(renderer,getres(BOOBS0_PNG),1))== 0 || 
      (snd[0]   = Mix_LoadWAV_RW(getres(SND0_OGG),1)) == 0   || 
      (bg[1]    = IMG_LoadTexture_RW(renderer,getres(BG1_PNG),1))   == 0 || 
      (girl[1]  = IMG_LoadTexture_RW(renderer,getres(GIRL1_PNG),1)) == 0 || 
      (boobs[1] = IMG_LoadTexture_RW(renderer,getres(BOOBS1_PNG),1)) == 0 || 
      (snd[1]   = Mix_LoadWAV_RW(getres(SND1_OGG),1)) == 0   || 
      (bg[2]    = IMG_LoadTexture_RW(renderer,getres(BG2_PNG),1))  == 0 || 
      (girl[2]  = IMG_LoadTexture_RW(renderer,getres(GIRL2_PNG),1)) == 0 || 
      (boobs[2] = IMG_LoadTexture_RW(renderer,getres(BOOBS2_PNG),1)) == 0 ||
      (snd[2] = Mix_LoadWAV_RW(getres(SND2_OGG),1)) == 0 ) { 
    fatal("Unable to load resources");
  }
  
}

void do_girl(SDL_Renderer* renderer, unsigned int ticks) {
  double girl_range = GIRL_MAX*2*((double)(cycle_length-cycle_ramp_up)/(double)cycle_length);
  SDL_Rect girldst;
  SDL_Rect boobsdst;
  double ramp_fract;
  girldst.x = 400;
  girldst.w = 400;
  girldst.h = 600;
  boobsdst.x = 400;
  boobsdst.w = 400;
  boobsdst.h = 600;
  
  if( cycle_ramp_up == cycle_ramp_down ) {
    girldst.y = GIRL_MAX;
    boobsdst.y = GIRL_MAX;
  }
  else if( ticks >= cycle_start+cycle_ramp_up ) {
    ramp_fract = (double)(ticks-cycle_start - cycle_ramp_up) / (double)(cycle_length - cycle_ramp_up);
    girldst.y =  (GIRL_MAX-girl_range) + girl_range*ramp_fract;
    boobsdst.y = girldst.y - BOOB_LAG*ramp_fract;
  }
  else if( ticks >= cycle_start+cycle_ramp_down ) {
    ramp_fract = (double)(ticks-cycle_start - cycle_ramp_down) / (double)(cycle_ramp_up - cycle_ramp_down);
    girldst.y = GIRL_MAX - (girl_range*ramp_fract);
    boobsdst.y = girldst.y + BOOB_LAG*ramp_fract;
  }
  else {
    girldst.y = GIRL_MAX;
    boobsdst.y = GIRL_MAX;
  }
  SDL_RenderCopy(renderer,girl[active_girl],0,&girldst);
  SDL_RenderCopy(renderer,boobs[active_girl],0,&boobsdst);
}

void do_audio(unsigned int ticks) {
  if( ticks == cycle_start ) {
    Mix_Volume(0, cycle_mag*128);
    Mix_PlayChannel(0,snd[active_girl],0);
  }
}

int main( int argc, char** argv ) {
  SDL_Event event;
  FPSmanager fps;
  int i;
  unsigned int ticks;
  SDL_Point mouse_pos = (SDL_Point){.x=0, .y=0};
  SDL_Rect dial_rects[3] = { (SDL_Rect){.x=100, .y=   0, .w=200, .h=200},
			     (SDL_Rect){.x=100, .y= 200, .w=200, .h=200},
			     (SDL_Rect){.x=100, .y= 400, .w=200, .h=200} };
  SDL_Rect next_rect = (SDL_Rect){.x=700, .y=0, .w=100, .h=55};
  int active_dial = -1;

  init_sdl();
  init_wave();
  load_resources();

  SDL_initFramerate(&fps);
  SDL_setFramerate(&fps,60);

  ticks = SDL_GetTicks();
  generate_wave(ticks);
  for(;;) {
    ticks = SDL_GetTicks();
    while( SDL_PollEvent(&event) ) {
      if( event.type == SDL_QUIT ) {
	SDL_Quit();
	return 0;
      }
      else if( event.button.type == SDL_MOUSEBUTTONDOWN &&
	       event.button.button == SDL_BUTTON_LEFT ) {
	for( i=0; i<3; i++ ) {
	  if( mouse_pos.x >= dial_rects[i].x &&
	      mouse_pos.x <  dial_rects[i].x+dial_rects[i].w &&
	      mouse_pos.y >= dial_rects[i].y &&
	      mouse_pos.y <  dial_rects[i].y+dial_rects[i].h ) {
	    active_dial = i;
	    break;
	  }
	}
	if( i == 3 ) {
	  if( mouse_pos.x >= next_rect.x &&
	      mouse_pos.x <  next_rect.x+next_rect.w &&
	      mouse_pos.y >= next_rect.y &&
	      mouse_pos.y <  next_rect.y+next_rect.h ) {
	    active_girl = (active_girl+1) % GIRL_COUNT;
	    
	  }
	}
      }
      else if( event.type == SDL_MOUSEBUTTONUP &&
	       event.button.button == SDL_BUTTON_LEFT ) {
	active_dial = -1;
      }
      else if( event.type == SDL_MOUSEMOTION ) {
        mouse_pos.x = event.motion.x;
        mouse_pos.y = event.motion.y;
	if( active_dial != -1 ) {
	  SDL_Point dial_center;
	  double angle;
	  dial_center.x = dial_rects[active_dial].x + dial_rects[active_dial].w/2;
	  dial_center.y = dial_rects[active_dial].y + dial_rects[active_dial].h/2;
	  if( mouse_pos.y > dial_center.y ) {
	    if( mouse_pos.x > dial_center.x ) {
	      angle = atan( (double)(mouse_pos.x-dial_center.x) / 
			    (double)(mouse_pos.y-dial_center.y) );
	      angle = 180.0 - (angle * 180 / PI);
	    }
	    else if( mouse_pos.x < dial_center.x ) {
	      angle = atan( (double)(dial_center.x-mouse_pos.x) / 
			    (double)(mouse_pos.y-dial_center.y) );
	      angle = -180.0 - (angle * -180 / PI);
	    }
	    else {
	      angle = 0.0;
	    }
	  }
	  else {
	    if( mouse_pos.x > dial_center.x ) {
	      angle = atan( (double)(dial_center.y-mouse_pos.y) / 
			    (double)(mouse_pos.x-dial_center.x) );
	      angle = 90.0 - (angle * 180.0 / PI);
	    }
	    else if( mouse_pos.x < dial_center.x ) {
	      angle = atan( (double)(dial_center.y-mouse_pos.y) / 
			    (double)(dial_center.x-mouse_pos.x) );
	      angle = -90 - (angle * -180.0 / PI);
	    }
	    else {
	      angle = 180.0;
	    }
	  }
	  if( angle < MIN_ANGLE ) { angle = MIN_ANGLE; }
	  if( angle > MAX_ANGLE ) { angle = MAX_ANGLE; }
	  dial_angles[active_dial] = angle;
	  generate_wave(ticks);
	}
      }
    }
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer,bg[active_girl],0,0);
    SDL_RenderCopy(renderer,hud,0,0);

    for( i=0; i<3; i++ ) {
      SDL_RenderCopyEx(renderer,dial,0,dial_rects+i,dial_angles[i],0,SDL_FLIP_NONE);
    }
    do_wave(ticks);
    do_girl(renderer,ticks);
    do_audio(ticks);
    SDL_RenderPresent(renderer);

    SDL_framerateDelay(&fps);
  }
   
  return 0;
}
