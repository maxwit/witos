#!/bin/sh

sed -i 's/\s\+$//' `find -type f -name "*.[ch]"`
