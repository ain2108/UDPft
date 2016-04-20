#!/bin/bash
hostname -I | cut -d ' ' -f1 > $1
hostname -I | cut -d ' ' -f3 >> $1
