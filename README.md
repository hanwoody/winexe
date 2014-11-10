## Running windows cmd from linux
To run windows cmd from linux box, there is one tool you could use, which is winexe. You can download the installer from here. There are 2 ways to install this tool:


#### 1. Use the preinstalled version.
Download from here
Unpack the bz2 file: # bunzip2 winexe-static-081123.bz2
Change mod to allow execute: # chmod +x winexe-static-081123
Make soft link in your /usr/local/bin: # ln -s winexe-static-081123 /usr/local/bin/winexe

#### 2. Compile from source
Install necessary packages (gcc, svn, *-devel....)
Get sources from here
Unpack the source file: # tar -xvjf winexe-source-081123.tar.bz2
Compile according to README file:
cd to unpacked tar.bz2 sources
./autogen.sh
./configure
make proto bin/winexe
Compiled file will be located in wmi/Samba/source/bin/winexe
Install winexe:
install -s wmi/Samba/source/bin/winexe /usr/local/bin/winexe

To use it is very simple:

    # winexe -U foo -W WORKGROUP -n FOO-PC //10.0.0.61 "cmd.exe"

where -U for username, -W for workgroup, -n for target machine netbios name, 10.0.0.61 is the ip address of the target machine and cmd.exe is to start windows command prompt.
Once connected, you will get command prompt like below:

Microsoft Windows [Version 5.2.3790]
(C) Copyright 1985-2003 Microsoft Corp.

C:\WINDOWS\system32>

To quit, just type exit at the windows command prompt.

That's all :)
