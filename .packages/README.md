# Package Cache Directory

This directory contains Toit packages that have been downloaded by
the Toit package management system.

Generally, the package manager is able to download these packages again. It
is thus safe to remove the content of this directory.

  simcom-cellular:
    url: github.com/toitware/simcom-cellular
    version: 0.1.9
    hash: 6179c73c3962c00bd5f2e56bef3664dbe82a97c1
    prefixes:
      cellular: cellular
      gnss_location: toit-gnss-location
      location: toit-location

        simcom_cellular:
    url: github.com/toitware/simcom-cellular
    version: ^0.1.9

      simcom_cellular: simcom-cellular