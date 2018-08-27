#ifndef PLAYER_H
#define PLAYER_H

__BEGIN_DECLS

void    player_init(void);
void    player_exit(void);
void    player_setdebugmode(void);

int    player_getduration(void);    /* duration in seconds */
int    player_getposition(void);    /* position in seconds */
int    player_getstatus(void);

int    player_play(const char *url);
void    player_stop(void);
void    player_togglepause(void);

/* 封装 ? 移动到播放列表控制 */
#include "mibrain_speak.h"
#include "mibrain_player.h"

int miBrain_audio_pause_handler(void);
void miBrain_audio_stop_handler(void);
void miBrain_audio_play_handler(const char *url, int offset);
void aivs_speak_handler(const char *url);

__END_DECLS

#endif
