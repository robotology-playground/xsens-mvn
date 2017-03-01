#!/bin/sh

set -e

#if OS is linux or is not set
if [ "$TRAVIS_OS_NAME" = linux -o -z "$TRAVIS_OS_NAME" ]; then
    sudo sh -c 'echo "deb http://www.icub.org/ubuntu trusty contrib/science" > /etc/apt/sources.list.d/icub.list'
    sudo apt-get update
    sudo apt-get -y --force-yes install -qq libeigen3-dev yarp 
elif [ "$TRAVIS_OS_NAME" = osx ]; then
    # we can remove xcpretty in Xcode 8.0
    gem install xcpretty
    brew update &> /dev/null
    brew tap robotology/cask
    brew tap homebrew/science
    brew install eigen yarp
fi
