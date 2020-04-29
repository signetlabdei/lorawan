#!/bin/bash

cd ../..
./waf configure
./waf sphinx
doxygen doc/doxygen.conf
cd lorawan-docs
rsync -rl ../doc/ ./
cp ../src/lorawan/README.md ./
git add .
git commit -m "Update API"
git push origin gh-pages
