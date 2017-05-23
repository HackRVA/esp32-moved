#ifndef PSMOVE_PROTOCALL_H
#define PSMOVE_PROTOCALL_H


 /**
 * PS Move API - An interface for the PS Move Motion Controller
 * Copyright (c) 2016 Thomas Perl <m@thp.io>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 **/

#define PSMOVE_BUFFER_SIZE 49
/*! Size of buffer for holding the extension device's data as reported by the Move */
#define PSMOVE_EXT_DATA_BUF_SIZE 5


enum PSMove_Request_Type {
    PSMove_Req_GetInput = 0x01,
    PSMove_Req_SetLEDs = 0x02,
    PSMove_Req_SetLEDPWMFrequency = 0x03,
    PSMove_Req_GetBTAddr = 0x04,
    PSMove_Req_SetBTAddr = 0x05,
    PSMove_Req_GetCalibration = 0x10,
    PSMove_Req_SetAuthChallenge = 0xA0,
    PSMove_Req_GetAuthResponse = 0xA1,
    PSMove_Req_GetExtDeviceInfo = 0xE0,
    PSMove_Req_SetExtDeviceInfo = 0xE0,
    PSMove_Req_SetDFUMode = 0xF2,
    PSMove_Req_GetFirmwareInfo = 0xF9,

    /**
     * Permanently set LEDs via USB
     *
     * Writing USB report 0xFA controls the LEDs. But unlike BT report
     * 0x02 this one keeps the sphere glowing as long as the USB cable
     * is connected, i.e. no refresh updates need to be send. Not sure
     * why that one exists. Might be useful for debugging though.
     *
     * TODO: Can't get this to work, but I could imagine it be useful for
     * things like notification apps that don't have to be running all the
     * time. Maybe it only works in specific controller firmware versions?
     *
     * http://lists.ims.tuwien.ac.at/pipermail/psmove/2013-March/000335.html
     * https://github.com/thp/psmoveapi/issues/55
     **/
    PSMove_Req_SetLEDsPermanentUSB = 0xFA,
};
 
typedef struct {
    unsigned char type; /* message type, must be PSMove_Req_SetLEDs */
    unsigned char _zero; /* must be zero */
    unsigned char r; /* red value, 0x00..0xff */
    unsigned char g; /* green value, 0x00..0xff */
    unsigned char b; /* blue value, 0x00..0xff */
    unsigned char rumble2; /* unknown, should be 0x00 for now */
    unsigned char rumble; /* rumble value, 0x00..0xff */
    unsigned char _padding[PSMOVE_BUFFER_SIZE-7]; /* must be zero */
} PSMove_Data_LEDs;

typedef struct {
    unsigned char type; /* message type, must be PSMove_Req_GetInput */
    unsigned char buttons1;
    unsigned char buttons2;
    unsigned char buttons3;
    unsigned char buttons4;
    unsigned char trigger; /* trigger value; 0..255 */
    unsigned char trigger2; /* trigger value, 2nd frame */
    unsigned char _unk7;
    unsigned char _unk8;
    unsigned char _unk9;
    unsigned char _unk10;
    unsigned char timehigh; /* high byte of timestamp */
    unsigned char battery; /* battery level; 0x05 = max, 0xEE = USB charging */
    unsigned char aXlow; /* low byte of accelerometer X value */
    unsigned char aXhigh; /* high byte of accelerometer X value */
    unsigned char aYlow;
    unsigned char aYhigh;
    unsigned char aZlow;
    unsigned char aZhigh;
    unsigned char aXlow2; /* low byte of accelerometer X value, 2nd frame */
    unsigned char aXhigh2; /* high byte of accelerometer X value, 2nd frame */
    unsigned char aYlow2;
    unsigned char aYhigh2;
    unsigned char aZlow2;
    unsigned char aZhigh2;
    unsigned char gXlow; /* low byte of gyro X value */
    unsigned char gXhigh; /* high byte of gyro X value */
    unsigned char gYlow;
    unsigned char gYhigh;
    unsigned char gZlow;
    unsigned char gZhigh;
    unsigned char gXlow2; /* low byte of gyro X value, 2nd frame */
    unsigned char gXhigh2; /* high byte of gyro X value, 2nd frame */
    unsigned char gYlow2;
    unsigned char gYhigh2;
    unsigned char gZlow2;
    unsigned char gZhigh2;
    unsigned char temphigh; /* temperature (bits 12-5) */
    unsigned char templow_mXhigh; /* temp (bits 4-1); magneto X (bits 12-9) */
    unsigned char mXlow; /* magnetometer X (bits 8-1) */
    unsigned char mYhigh; /* magnetometer Y (bits 12-5) */
    unsigned char mYlow_mZhigh; /* magnetometer: Y (bits 4-1), Z (bits 12-9) */
    unsigned char mZlow; /* magnetometer Z (bits 8-1) */
    unsigned char timelow; /* low byte of timestamp */
    unsigned char extdata[PSMOVE_EXT_DATA_BUF_SIZE]; /* external device data (EXT port) */
} PSMove_Data_Input;


#endif