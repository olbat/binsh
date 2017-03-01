# binsh - obfuscate shell scripts

## Overview
This tool allows to obfuscate shell scripts by embedding them in a compiled program, encrypting and passphrase-protect them.

The script is included in the compiled C program as a constant, when you run the program the script is executed using the specified shell's command (by default: `/bin/sh` `-c` `--`).

The running environment (arguments, environement variables, fds) is "forwarded" to the script.


## Usage
To build the program that embeds the script
```
build.sh <script> <passphrase>|- [<output>] [<shell> <exec> <command> <...>]"
```

To run the program that embeds the script (compiled as `binsh`)
```
./binsh <passphrase>|- [<script> <arguments> <...>]
```


## How does it work
The main idea is not to be able to determine what does the script do just by opening the script file.

First of all, the script is included as a constant in a compiled program so it's harder to understand what it does by just opening the file.

The script is encrypted using a simple key-based symetric encryption algorithm before the compilation so it's not possible to determine what the binary file does using softwares such as `strings`.

In the end, the script is decrypted at run time using the key in order to avoid the binary to be analysed using softwares such a `strace` (without the key you can't run the script).


## Examples

### Compile then run a script using a passphrase
```bash
./build.sh script.sh p4ssphras3 script
# Build of 'script' successful

./script p4ssphras3 --opt 123
# ...
```

### Compile then run a script using a key file
```bash
dd if=/dev/urandom of=keyfile bs=512 count=1
# 512 bytes (512 B) copied, ...

./build.sh script.sh - script < keyfile
# Build of 'script' successful

./script - --opt 123 < keyfile
# ...
```

### Compile then run a Perl script
```bash
cat <<'EOF' > script.pl
use strict;
print 'ARGV: ["' . join('", "', @ARGV) . "\"]\n";
print 'ENV["TEST"]: ' . $ENV{"TEST"} . "\n";
EOF

./build.sh script.pl p4ssphras3 script /usr/bin/perl -w -- -
# Build of 'script' successful

TEST=123 ./script p4ssphras3 --abc "def hij" --klm
#ARGV: ["--abc", "def hij", "--klm"]
#ENV["TEST"]: 123
```

### Compile then run a Python script
```bash
cat <<'EOF' > script.py
import sys
import os
print 'ARGV: ' + repr(sys.argv[1:])
print 'ENV["TEST"]: ' + os.environ.get('TEST')
EOF

./build.sh script.py p4ssphras3 script /usr/bin/python -Wall -
# Build of 'script' successful

TEST=123 ./script p4ssphras3 --abc "def hij" --klm
#ARGV: ['--abc', 'def hij', '--klm']
#ENV["TEST"]: 123
```

### Compile then run a Ruby script
```bash
cat <<'EOF' > script.rb
puts "ARGV: " + ARGV.inspect
puts 'ENV["TEST"]: ' + ENV['TEST']
EOF

./build.sh script.rb p4ssphras3 script /usr/bin/ruby -w -- -
# Build of 'script' successful

TEST=123 ./script p4ssphras3 --abc "def hij" --klm
#ARGV: ["--abc", "def hij", "--klm"]
#ENV["TEST"]: 123
```
