## Group Info
* Group Numebr: __32__
* Port: 5500 + 32 = __5532__
* OS2 Folder: __/scratch/spring2018/32/__

## Qemu Command

``` bash
qemu-system-i386 -gdb tcp::5532 -S -nographic -kernel <kernal_path> -drive file=core-image-lsb-sdk-qemux86.ext4,if=virtio -enable-kvm -net none -usb -localtime --no-reboot --append "root=/dev/vda rw console=ttyS0 debug"
```

The kernal path for testing vm initialization is 'bzImage-qemux86.bin' when inside of our folder.  Otherwise, when building the kernal the binary will be located at '?'.

## HW1 Command List

### Build Kernal
1.  Login to the os2 server. Two terminals are nessacary.
2.  `mkdir /scratch/spring2018/32` - creates our group folder on os2.
3.  `cd /scratch/spring2018/32` - change into newly created group folder.
4.  `cp /scratch/opt/poky/1.8/environment-setup-i586-poky-linux .` - copy the environment file that will be used to set up our environment.
5.  `source environment-setup-i586-poky-linux` - set up our terminal environment.
6.  `git clone --branch v3.19.2 git://git.yoctoproject.org/linux-yocto-3.19` - clone the proper tag from the provided git repo.
7.  `cd linux-yocto-3.19/` - change directories into the newly cloned repo.
8.  `git checkout -b os2` - checkout a new branch that will serve as our base branch for future homework assignments.
9. `cp /scratch/files/config-3.19.2-yocto-standard .config` - copy the provided config file into the source tree.
10. `make menuconfig` - opens menuconfig window for the project. Go into general settings and then change 'Local Version' to '-group32-hw1', save to '.config' then exit.
11. `make clean` - cleans out any data from pervious builds (not nessacary from first time building).
12. `make -j4 all > ../build.out` - this will begin the building of the kernal in the current terminal pipeing the output in the build.out file.
13. From the other terminal, with its working directory at the base of the kernal source tree run: `tail -f ../build.out` - this views the kernal progress.

### Run Qemu VM
1.  Login to the os2 server. Two terminals are nessacary.
2.  `cd /scratch/spring2018/32` - change into group folder.
3.  `source environment-setup-i586-poky-linux` - setup the terminal environment for the vm.
4.  `cp /scratch/files/bzImage-qemux86.bin .` - copy the provided kernel to the group directory.
5.  `cp /scratch/files/core-image-lsb-sdk-qemux86.ext4 .` - copy the file system file to our group folder.
6.  `qemu-system-i386 -gdb tcp::5532 -S -nographic -kernel bzImage-qemux65.bin -drive file=core-image-lsb-sdk-qemux86.ext4,if=virtio -enable-kvm -net none -usb -localtime --no-reboot --append "root=/dev/vda rw console=ttyS0 debug"` - launches the vm in a halted state (freezing the current terminal).
7.  `gdb -tui` - open gdb in tui mode.
8.  `target remote:5532` - select the remote vm target on the port set in the vm command.
9.  `continue` - continues the execution of the vm (on the other terminal).
10. Once displayed with the login prompt, on the other terminal: `root` - this is the password, press enter and then you are logged into the vm as the password is empty.
