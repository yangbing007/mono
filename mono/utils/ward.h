#ifndef MONO_UTILS_WARD_H
#define MONO_UTILS_WARD_H

#ifdef __WARD__
#define MONO_PERMIT(...) __attribute__ ((ward (__VA_ARGS__)))
#else
#define MONO_PERMIT(...)
#endif

/* Add Ward permissions for external functions (e.g., libc, glib) here. */

#ifdef __WARD__
#define BEGIN_NO_CHECKPOINT() do { mono_need_no_checkpoint_begin(); do {} while (0)
#define END_NO_CHECKPOINT() mono_need_no_checkpoint_end(); } while (0)
static inline MONO_PERMIT(need(coop_can_checkpoint),revoke(coop_can_checkpoint))
void mono_need_no_checkpoint_begin () { }

static inline MONO_PERMIT(waive(coop_can_checkpoint),deny(coop_can_checkpoint),grant(coop_can_checkpoint))
void mono_need_no_checkpoint_end () { }
#else
#define BEGIN_NO_CHECKPOINT() do { do {} while (0)
#define END_NO_CHECKPOINT() } while (0)
#endif

#endif
