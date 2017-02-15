import os
import glob

devices = glob.glob('/sys/bus/usb/devices/*')

for device in devices:
  try:
    # get vendor ID
    f = open(device + '/idVendor')
    try:
      id_vendor = f.readlines()[0]
    finally:
      f.close()
    # get product ID
    f = open(device + '/idProduct')
    try:
      id_product = f.readlines()[0]
    finally:
      f.close()
    wakeup = 'N/A'
    # get wakeup state
    f = open(device + '/power/wakeup')
    try:
      wakeup = f.readlines()[0]
      wakeup = wakeup[:len(wakeup)-1]
    finally:
      f.close()
  except:
    pass
  print('Device ' + id_vendor[0:4] + ':' + id_product[0:4] + ' at ' + device + ' (wakeup ' + wakeup + ')')