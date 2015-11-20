#!/bin/bash

# The envirinment set by create-ubuntu-environmens sscript is trusty, so we should match it.
# Look at debian/gbp.conf for default optiuons sent to gbp.

git commit -a -m 'Preparing for pbuild' && git push
# DIST=trusty gbp buildpackage
gbp buildpackage --git-pbuilder --git-dist=trusty

# This should produce .deb output in export directory
