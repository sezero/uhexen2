/* Automatically generated header (sfdc 1.4)! Do not edit! */

#ifndef _INLINE_AHI_H
#define _INLINE_AHI_H

#ifndef _SFDC_VARARG_DEFINED
#define _SFDC_VARARG_DEFINED
#ifdef __HAVE_IPTR_ATTR__
typedef APTR _sfdc_vararg __attribute__((iptr));
#else
typedef ULONG _sfdc_vararg;
#endif /* __HAVE_IPTR_ATTR__ */
#endif /* _SFDC_VARARG_DEFINED */

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef AHI_BASE_NAME
#define AHI_BASE_NAME AHIBase
#endif /* !AHI_BASE_NAME */

#define AHI_AllocAudioA(___tagList) \
	LP1(0x2a, struct AHIAudioCtrl *, AHI_AllocAudioA , struct TagItem *, ___tagList, a1,\
	, AHI_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define AHI_AllocAudio(___tag1, ...) \
	({_sfdc_vararg _tags[] = { ___tag1, __VA_ARGS__ }; AHI_AllocAudioA((struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define AHI_FreeAudio(___AudioCtrl) \
	LP1NR(0x30, AHI_FreeAudio , struct AHIAudioCtrl *, ___AudioCtrl, a2,\
	, AHI_BASE_NAME)

#define AHI_ControlAudioA(___AudioCtrl, ___tagList) \
	LP2(0x3c, ULONG, AHI_ControlAudioA , struct AHIAudioCtrl *, ___AudioCtrl, a2, struct TagItem *, ___tagList, a1,\
	, AHI_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define AHI_ControlAudio(___AudioCtrl, ___tag1, ...) \
	({_sfdc_vararg _tags[] = { ___tag1, __VA_ARGS__ }; AHI_ControlAudioA((___AudioCtrl), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define AHI_SetVol(___Channel, ___Volume, ___Pan, ___AudioCtrl, ___Flags) \
	LP5NR(0x42, AHI_SetVol , UWORD, ___Channel, d0, Fixed, ___Volume, d1, sposition, ___Pan, d2, struct AHIAudioCtrl *, ___AudioCtrl, a2, ULONG, ___Flags, d3,\
	, AHI_BASE_NAME)

#define AHI_SetFreq(___Channel, ___Freq, ___AudioCtrl, ___Flags) \
	LP4NR(0x48, AHI_SetFreq , UWORD, ___Channel, d0, ULONG, ___Freq, d1, struct AHIAudioCtrl *, ___AudioCtrl, a2, ULONG, ___Flags, d2,\
	, AHI_BASE_NAME)

#define AHI_SetSound(___Channel, ___Sound, ___Offset, ___Length, ___AudioCtrl, ___Flags) \
	LP6NR(0x4e, AHI_SetSound , UWORD, ___Channel, d0, UWORD, ___Sound, d1, ULONG, ___Offset, d2, LONG, ___Length, d3, struct AHIAudioCtrl *, ___AudioCtrl, a2, ULONG, ___Flags, d4,\
	, AHI_BASE_NAME)

#define AHI_SetEffect(___Effect, ___AudioCtrl) \
	LP2(0x54, ULONG, AHI_SetEffect , APTR, ___Effect, a0, struct AHIAudioCtrl *, ___AudioCtrl, a2,\
	, AHI_BASE_NAME)

#define AHI_LoadSound(___Sound, ___Type, ___Info, ___AudioCtrl) \
	LP4(0x5a, ULONG, AHI_LoadSound , UWORD, ___Sound, d0, ULONG, ___Type, d1, APTR, ___Info, a0, struct AHIAudioCtrl *, ___AudioCtrl, a2,\
	, AHI_BASE_NAME)

#define AHI_UnloadSound(___Sound, ___Audioctrl) \
	LP2NR(0x60, AHI_UnloadSound , UWORD, ___Sound, d0, struct AHIAudioCtrl *, ___Audioctrl, a2,\
	, AHI_BASE_NAME)

#define AHI_NextAudioID(___Last_ID) \
	LP1(0x66, ULONG, AHI_NextAudioID , ULONG, ___Last_ID, d0,\
	, AHI_BASE_NAME)

#define AHI_GetAudioAttrsA(___ID, ___Audioctrl, ___tagList) \
	LP3(0x6c, BOOL, AHI_GetAudioAttrsA , ULONG, ___ID, d0, struct AHIAudioCtrl *, ___Audioctrl, a2, struct TagItem *, ___tagList, a1,\
	, AHI_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define AHI_GetAudioAttrs(___ID, ___Audioctrl, ___tag1, ...) \
	({_sfdc_vararg _tags[] = { ___tag1, __VA_ARGS__ }; AHI_GetAudioAttrsA((___ID), (___Audioctrl), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define AHI_BestAudioIDA(___tagList) \
	LP1(0x72, ULONG, AHI_BestAudioIDA , struct TagItem *, ___tagList, a1,\
	, AHI_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define AHI_BestAudioID(___tag1, ...) \
	({_sfdc_vararg _tags[] = { ___tag1, __VA_ARGS__ }; AHI_BestAudioIDA((struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define AHI_AllocAudioRequestA(___tagList) \
	LP1(0x78, struct AHIAudioModeRequester *, AHI_AllocAudioRequestA , struct TagItem *, ___tagList, a0,\
	, AHI_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define AHI_AllocAudioRequest(___tag1, ...) \
	({_sfdc_vararg _tags[] = { ___tag1, __VA_ARGS__ }; AHI_AllocAudioRequestA((struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define AHI_AudioRequestA(___Requester, ___tagList) \
	LP2(0x7e, BOOL, AHI_AudioRequestA , struct AHIAudioModeRequester *, ___Requester, a0, struct TagItem *, ___tagList, a1,\
	, AHI_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define AHI_AudioRequest(___Requester, ___tag1, ...) \
	({_sfdc_vararg _tags[] = { ___tag1, __VA_ARGS__ }; AHI_AudioRequestA((___Requester), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define AHI_FreeAudioRequest(___Requester) \
	LP1NR(0x84, AHI_FreeAudioRequest , struct AHIAudioModeRequester *, ___Requester, a0,\
	, AHI_BASE_NAME)

#define AHI_PlayA(___Audioctrl, ___tagList) \
	LP2NR(0x8a, AHI_PlayA , struct AHIAudioCtrl *, ___Audioctrl, a2, struct TagItem *, ___tagList, a1,\
	, AHI_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define AHI_Play(___Audioctrl, ___tag1, ...) \
	({_sfdc_vararg _tags[] = { ___tag1, __VA_ARGS__ }; AHI_PlayA((___Audioctrl), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define AHI_SampleFrameSize(___SampleType) \
	LP1(0x90, ULONG, AHI_SampleFrameSize , ULONG, ___SampleType, d0,\
	, AHI_BASE_NAME)

#define AHI_AddAudioMode(___AHIPrivate) \
	LP1(0x96, ULONG, AHI_AddAudioMode , struct TagItem *, ___AHIPrivate, a0,\
	, AHI_BASE_NAME)

#define AHI_RemoveAudioMode(___AHIPrivate) \
	LP1(0x9c, ULONG, AHI_RemoveAudioMode , ULONG, ___AHIPrivate, d0,\
	, AHI_BASE_NAME)

#define AHI_LoadModeFile(___AHIPrivate) \
	LP1(0xa2, ULONG, AHI_LoadModeFile , STRPTR, ___AHIPrivate, a0,\
	, AHI_BASE_NAME)

#endif /* !_INLINE_AHI_H */
