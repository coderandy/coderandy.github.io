#!/bin/bash
arp-scan --retry=8 --ignoredups -I eth0 --localnet
