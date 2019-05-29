#ifndef QT_DIGIPOT_H_
#define QT_DIGIPOT_H_

#include <stdbool.h>
#include "QT_AD5292.h"
#include "Common/QT_COM_common.h"
#include "OBCInterface/QT_OBC_Interface.h"

bool QT_DIGIPOT_setGain(float gain);

byte* QT_DIGIPOT_getValue();

void QT_DIGIPOT_init();

#endif /* QT_DIGIPOT_H_ */
