/*****************************************************************************
** $Source: /cvsroot/bluemsx/blueMSX/Src/Input/MsxJoystickDevice.h,v $
**
** $Revision: 1.3 $
**
** $Date: 2008/03/30 18:38:40 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************
*/
#ifndef MSX_JOYSTICK_DEVICE_H
#define MSX_JOYSTICK_DEVICE_H

#include "../Common/MsxTypes.h"

// Base class for MSX Joystick devices. 

typedef struct {
    UInt8 (*read)(void*);
    void  (*write)(void*, UInt8);
    void  (*destroy)(void*);
    void  (*reset)(void*);
    void  (*saveState)(void*);
    void  (*loadState)(void*);
} MsxJoystickDevice;


#endif 
