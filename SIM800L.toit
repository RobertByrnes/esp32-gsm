// Copyright (C) 2022 Toitware ApS. All rights reserved.
// Use of this source code is governed by an MIT-style license that can be
// found in the LICENSE file.

import at
import bytes
import gpio
import log
import uart

import .simcom_cellular

/**
Driver for SIM800L modem.
*/
class SIM800L extends SimcomCellular:
  pwrkey/gpio.Pin?
  rstkey/gpio.Pin?

  constructor uart/uart.Port --logger=log.default --.pwrkey=null --.rstkey=null --is_always_online/bool:
    super
      uart
      --logger=logger
      --preferred_baud_rate=115200
      --use_psm=not is_always_online

  on_connected_ session/at.Session:
    // Attach to network.
    // session.set "+QICSGP" [cid_]
    session.set "+CGATT" [1]
    // session.set "+QIACT" [cid_]

    // Set to multi IP
    session.set "+CIPMUX" [1]

    // Put in "quick send" mode (thus no extra "Send OK")
    session.set "+CIPQSEND" [1]

    // Set to get data manually
    session.set "+CIPRXGET" [1]


      // Attach to GPRS
    // sendAT(GF("+CGATT=1"));
    // if (waitResponse(60000L) != 1) { return false; }

    // Set to multi-IP
    // sendAT(GF("+CIPMUX=1"));
    // if (waitResponse() != 1) { return false; }

    // Put in "quick send" mode (thus no extra "Send OK")
    // sendAT(GF("+CIPQSEND=1"));
    // if (waitResponse() != 1) { return false; }

    // Set to get data manually
    // sendAT(GF("+CIPRXGET=1"));
    // if (waitResponse() != 1) { return false; }

    // Start Task and Set APN, USER NAME, PASSWORD
    // sendAT(GF("+CSTT=\""), apn, GF("\",\""), user, GF("\",\""), pwd, GF("\""));
    // if (waitResponse(60000L) != 1) { return false; }

    // // Bring Up Wireless Connection with GPRS or CSD
    // sendAT(GF("+CIICR"));
    // if (waitResponse(60000L) != 1) { return false; }

    // // Get Local IP Address, only assigned after connection
    // sendAT(GF("+CIFSR;E0"));
    // if (waitResponse(10000L) != 1) { return false; }

    // // Configure Domain Name Server (DNS)
    // sendAT(GF("+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\""));
    // if (waitResponse() != 1) { return false; }

  on_reset session/at.Session:
    session.set "+CFUN" [1, 1]
 
  power_on -> none:
    if pwrkey:
      pwrkey.set 1
      sleep --ms=150
      pwrkey.set 0
 
  power_off -> none:
    if pwrkey:
      pwrkey.set 1
      sleep --ms=650
      pwrkey.set 0

  reset -> none:
    if rstkey:
      rstkey.set 1
      sleep --ms=150
      rstkey.set 0
 
  recover_modem -> none:
    if rstkey:
      reset
    else:
      power_off
