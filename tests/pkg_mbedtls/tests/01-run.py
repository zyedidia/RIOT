#!/usr/bin/env python3

# Copyright (C) 2020 HAW Hamburg
#
# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import sys
from testrunner import run


def testfunc(child):
    child.expect_exact('SHA-224 test #1: passed')
    child.expect_exact('SHA-224 test #2: passed')
    child.expect_exact('SHA-224 test #3: passed')
    child.expect_exact('SHA-256 test #1: passed')
    child.expect_exact('SHA-256 test #2: passed')
    child.expect_exact('SHA-256 test #3: passed')
    child.expect_exact('ENTROPY test: passed')


if __name__ == "__main__":
    sys.exit(run(testfunc))
