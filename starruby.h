#ifndef STARRUBY_H
#define STARRUBY_H

#ifdef DEFINE_STARRUBY_EXTERN
#define STARRUBY_EXTERN
#else
#define STARRUBY_EXTERN extern
#endif

void Init_starruby(void);

#endif
