import usb.core

dev = usb.core.find(idVendor=0x0483, idProduct=0x5726)

if dev is None:
	raise Exception('Could not find device')

###
for cfg in dev:
	print str(cfg.bConfigurationValue)
	for intf in cfg:
		print '\t'+ str(intf.bInterfaceNumber) + ', ' + str(intf.bAlternateSetting)
		for ep in intf:
			print '\t\t' + str(ep.bEndpointAddress)
