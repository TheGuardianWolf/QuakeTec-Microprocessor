/*
 * QT_LPMain.h
 *
 *  Created on: 20/02/2019
 *      Author: james
 */

#ifndef QT_LPMAIN_H_
#define QT_LPMAIN_H_

typedef struct //function_flags
{
    uint16_t probe_deploying;
} f_flags;

extern volatile f_flags F_FLAGS;

/** Set to true if a command should exit as soon as possible. Commands must not reset this flag, and should leave their systems in a 'safe' state. */
extern volatile bool exitCommand;

#endif /* QT_LPMAIN_H_ */
