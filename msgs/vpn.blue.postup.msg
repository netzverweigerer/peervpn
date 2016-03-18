[postup] Thu Mar 17 14:46:43 CET 2016: /home/armin/git/peervpn/configs/vpn.blue.postup execution initiated.

[postup] Thu Mar 17 14:46:43 CET 2016: Sleeping for 5 seconds...
[postup] Thu Mar 17 14:46:48 CET 2016: Resuming operation...

[postup] Thu Mar 17 14:46:48 CET 2016: Removing stale IPv6 default route entry... [postup] Thu Mar 17 14:46:48 CET 2016: done.

[postup] Thu Mar 17 14:46:48 CET 2016: Adding new IPv6 default route (via 2a00:1158:2:a710::1)... 
[postup] Thu Mar 17 14:46:48 CET 2016: done.

[postup] Thu Mar 17 14:46:48 CET 2016: Routing-Table: 
2a00:1158:2:a710::/60 dev vpn.blue  proto kernel  metric 256  pref medium
fe80::/64 dev wlan0  proto kernel  metric 256  pref medium
fe80::/64 dev vpn.blue  proto kernel  metric 256  pref medium
default via 2a00:1158:2:a710::1 dev vpn.blue  metric 1024  pref medium

[postup] Thu Mar 17 14:46:48 CET 2016: [postup] Quitting... 
