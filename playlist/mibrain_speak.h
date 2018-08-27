

#ifndef	__MIBRAIN_SPEAK_H__
#define	__MIBRAIN_SPEAK_H__


#ifdef __cplusplus
extern "C" {
#endif

void aivs_speak_init(void);
void aivs_speak_fini(void);

int aivs_speak_on_work(void);
void avis_speak_on_finished(void);

int aivs_speak_start(char *url);

#ifdef __cplusplus
}
#endif

#endif
