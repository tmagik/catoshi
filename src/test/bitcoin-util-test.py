#!/usr/bin/env python
# Copyright 2014 BitPay, Inc.
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
from __future__ import division,print_function,unicode_literals
import os
import sys
import bctest
import buildenv
import argparse
import logging

help_text="""Test framework for bitcoin utils.

Runs automatically during `make check`.

Can also be run manually from the src directory by specifiying the source directory:

test/bitcoin-util-test.py --src=[srcdir]
"""


if __name__ == '__main__':
    try:
        srcdir = os.environ["srcdir"]
        verbose = False
    except:
        parser = argparse.ArgumentParser(description=help_text)
        parser.add_argument('-s', '--srcdir')
        parser.add_argument('-v', '--verbose', action='store_true')
        args = parser.parse_args()
        srcdir = args.srcdir
        verbose = args.verbose

    if verbose:
        level = logging.DEBUG
    else:
        level = logging.ERROR
    formatter = '%(asctime)s - %(levelname)s - %(message)s'
    # Add the format/level to the logger
    logging.basicConfig(format = formatter, level=level)

    bctest.bctester(srcdir + "/test/data", "bitcoin-util-test.json", buildenv)
