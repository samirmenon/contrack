#!/bin/sh
for file in $(find ./ -type f -name "*.h" -o -name "*.c" -o -name "*.cpp"); do
   tr -d '\r' <$file >temp.$$ && mv temp.$$ $file
done
