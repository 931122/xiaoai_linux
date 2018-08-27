#!/bin/bash
export LD_LIBRARY_PATH=$PWD/sdk/lib:$PWD/wakeup/lib:/usr/local/lib:$LD_LIBRARY_PATH


[ -f ./out/bin/asr_example ] && {
./out/bin/asr_example $1
}


[ -f ./out/bin/demo ] && {
./out/bin/demo $1
}


[ -f ./out/bin/signal_trigger ] && {
./out/bin/signal_trigger $1
}


[ -f ./out/bin/wakeup_trigger ] && {
./out/bin/wakeup_trigger $1
}


