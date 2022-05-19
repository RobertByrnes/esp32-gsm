// Copyright (C) 2022 Toitware ApS.
// Use of this source code is governed by a Zero-Clause BSD license that can
// be found in the EXAMPLES_LICENSE file.

/**
This example demonstrates how to create a Sara R4 driver and connect it to a
  cellular network.

The example resets the modem before connecting to remove any unexpected state
  before connecting. However, this makes the connection time fairly long.
*/
import cellular
import gpio
import http
import http.client show Client
import log
import net
import uart
import simcom_cellular.SIM800L show SIM800L


APN ::= ["giffgaff.com","gg","p"]

// SIM800 pins
// define MODEM_RST            5                      // Reset pin
// define MODEM_PWKEY          4                      // Enable pin
// define MODEM_POWER_ON       23                     // Power pin
// define MODEM_TX             27                     // Transmit pin
// define MODEM_RX             26                     // Receive pin
// define I2C_SDA              21                     // Serial data
// define I2C_SCL              22                     // Serial clock

TX_PIN_NUM ::= 27
RX_PIN_NUM ::= 26
PWR_ON_NUM ::= 4
BAUD_RATE ::= 115200

logger ::= log.default

main:
  driver := create_driver

  if not connect driver: return

  network_interface := driver.network_interface
  visit_google network_interface
  driver.close

create_driver -> SIM800L:
  pwr_on :=  gpio.Pin PWR_ON_NUM
  pwr_on.config --output --open_drain
  pwr_on.set 1
  tx := gpio.Pin TX_PIN_NUM
  rx := gpio.Pin RX_PIN_NUM

  port := uart.Port --tx=tx --rx=rx --baud_rate=BAUD_RATE

  return SIM800L port --pwrkey=(gpio.InvertedPin pwr_on) --logger=log.default --is_always_online=false

reset driver:
  driver.wait_for_ready
  driver.reset

connect driver/cellular.Cellular -> bool:
  logger.info "WAITING FOR MODULE..."
  driver.wait_for_ready
  logger.info "model: $driver.model"
  logger.info "version $driver.version"
  logger.info "iccid: $driver.iccid"
  logger.info "CONFIGURING..."
  driver.configure APN --bands=BANDS --rats=RATS
  logger.info "ENABLING RADIO..."
  driver.enable_radio
  logger.info "CONNECTING..."
  try:
    dur := Duration.of:
      driver.connect
    logger.info "CONNECTED (in $dur)"
  finally: | is_exception _ |
    if is_exception:
      critical_do:
        driver.close
        logger.info "CONNECTION FAILED"
        return false
  return true

visit_google network_interface/net.Interface:
  host := "www.google.com"

  client := Client network_interface

  response := client.get host "/"

  bytes := 0
  while data := response.body.read:
    bytes += data.size

  logger.info "Read $bytes bytes from http://$host/"
