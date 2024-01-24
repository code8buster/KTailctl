#!/bin/bash
set -euf -o pipefail

go install -v github.com/go-critic/go-critic/cmd/gocritic@latest

cd tailwrap
~/go/bin/gocritic check .