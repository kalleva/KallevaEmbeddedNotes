To capture UDP packets to .pcap file I use tcpdump:

```sudo tcpdump -i eth0 -nn -s 65535 -w devices_log.pcap port 10001 and udp```

Analysis of the .pcap file can by done with Wireshark.
Recently I looked for UDP packets from some NBIoT devices sending data to UDP server.
Each packet contained IMEI of a NBIoT modem that was used in a device.
So for filtering packets from that particular device I created following
**display filter** in Wireshark:

```frame contains 0x:xx:xx:xx:xx:xx:xx```

Where instead of ```x``` I used lowercase hex symbols.