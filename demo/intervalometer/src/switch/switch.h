
/* ------------------------------------------------------------------------ */
/** @file  switch.h
  * @date   04/11/2015
  * @author Rafa García.                                                    */
/* ------------------------------------------------------------------------ */

#ifndef _SWITCH_
#define _SWITCH_

/** @defgroup switch Switch Ctrl
  * @{ */

/** Set a callback funcion invoked when button is pressed. */
void switch_setFunc( void(*func)(void) );

/** Thread that controls the switch state. */
void switch_task( void* param );

/** @} */

#endif /* _SWITCH_ */