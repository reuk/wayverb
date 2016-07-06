#!/bin/sh
cat $1 | ( echo "static const unsigned char data[] = {"; xxd -i; echo "};" ) > $2