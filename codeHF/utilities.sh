#!/bin/bash

# Utilities

# Message formatting
function MsgStep { echo -e "\n\e[1;32m$@\e[0m"; }
function MsgSubStep { echo -e "\e[1m$@\e[0m"; }
function MsgWarn { echo -e "\e[1;36m$@\e[0m"; }
function MsgErr { echo -e "\e[1;31m$@\e[0m"; }
