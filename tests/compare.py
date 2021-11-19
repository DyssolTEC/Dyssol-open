#!/usr/bin/env python3
#
# Copyright 2021, Dyssol Development Team
#           2021, Institute for mineral processing machines and recycling systems technology
#                 Technical University Bergakademie Freiberg
#
# BSD 3-Clause License
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#
# This script compares two files with numerical simulation results and returns an error if the difference is greater than tolerance.

import argparse
import os.path
import sys

if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description='compares two files with numerical simulation results and returns an error if the difference is greater than tolerance.')
    parser.add_argument('reference', action="store",
                        help='reference file')
    parser.add_argument('compare', action="store",
                        help='compare file')
    parser.add_argument('-t', '--tolerance', action="store", default=1e-3, dest="tolerance",
                        help='numerical tolerance')

    results = parser.parse_args()

    referencefile = results.reference
    comparefile = results.compare
    tolerance = float(results.tolerance)

    if (not(os.path.isfile(referencefile))):
        sys.exit(f"Reference file {referencefile} does not exist")

    if (not(os.path.isfile(comparefile))):
        sys.exit(f"compare file {comparefile} does not exist")

    print(f"Used tolerance is {tolerance}")

    linesReference = []

    with open(referencefile) as r:
        linesReference = r.readlines()

    linesCompare = []
    with open(comparefile) as c:
        linesCompare = c.readlines()

    if (len(linesCompare) != len(linesReference)):
        sys.exit(
            f"Number of lines in reference file ({len(linesReference)}) does not match the number of lines in compare file ({len(linesCompare)})!")

    for l in range(len(linesReference)):
        lr = linesReference[l].strip().split(" ")
        lc = linesCompare[l].strip().split(" ")
        if (len(lr) != len(lc)):
            sys.exit(
                f"Number of words in the line {l+1} in reference file ({len(lr)}) does not match the number of words in compare file ({len(lc)})!")

        for c in range(len(lr)):
            wr = lr[c].strip()
            wc = lc[c].strip()
            if (wr == wc):
                # Files are identical
                continue
            else:
                # Try to convert data to float and compare again
                try:
                    wrf = float(wr)
                    wrc = float(wc)
                except ValueError:
                    sys.exit(
                        f"Words number {c+1} in line {l+1} differ in reference ({wr}) and in compare ({wc}) files!")

                calculatedTolerance = abs(wrf - wrc)/min(abs(wrf), abs(wrc))
                if (calculatedTolerance > tolerance):
                    sys.exit(
                        f"Calculated tolerance ({calculatedTolerance}) is higher as required ({tolerance}), word number {c+1} in line {l+1}: reference ({wr}) and compare ({wc})!")

    print(f"Files {referencefile} and {comparefile} are identical")
    sys.exit(0)
