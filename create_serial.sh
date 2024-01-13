#!/bin/sh
socat pty,raw,echo=0,link=/tmp/dev/ttyS20 pty,raw,echo=0,link=/tmp/dev/ttyS21
