## Group Info
* Group Numebr: __32__
* Port: 5500 + 32 = __5532__
* OS2 Folder: __/scratch/spring2018/32/__

## Qemu Command

``` bash
qemu-system-i386 -gdb tcp::5532 -S -nographic -kernel <kernal_path> \
                 -drive file=core-image-lsb-sdk-qemux86.ext4,if=virtio \
                 -enable-kvm -net none -usb -localtime --no-reboot --append \
                 "root=/dev/vda rw console=ttyS0 debug"
```

The kernal path for testing vm initialization is 'bzImage-qemux86.bin' when inside of our folder.  Otherwise, when building the kernal the binary will be located at 'linux-yocto-3.19/arch/x86/boot/bzImage'.

## HW1 Command List

### Setup (from scratch)
1.  Login to the os2 server.
2.  `mkdir /scratch/spring2018/32` - creates our group folder on os2.
3.  `cd /scratch/spring2018/32` - change into newly created group folder.
4.  `cp /scratch/opt/poky/1.8/environment-setup-i586-poky-linux .` - copy the environment file that will be used to set up our environment.
5.  `cp /scratch/files/bzImage-qemux86.bin .` - copy the provided kernel to the group directory.
6.  `cp /scratch/files/core-image-lsb-sdk-qemux86.ext4 .` - copy the file system file to our group folder.
7.  `source environment-setup-i586-poky-linux` - set up our terminal environment.
8.  `git clone --branch v3.19.2 git://git.yoctoproject.org/linux-yocto-3.19` - clone the proper tag from the provided git repo.
9.  `cd linux-yocto-3.19/` - change directories into the newly cloned repo.
10. `git checkout -b os2` - checkout a new branch that will serve as our base branch for future homework assignments.
11. `cp /scratch/files/config-3.19.2-yocto-standard .config` - copy the provided config file into the source tree.
12. `make menuconfig` - opens menuconfig window for the project. Go into general settings and then change 'Local Version' to '-group32-hw1', save to '.config' then exit.

### Setup Kernel (for new assignment)
1.  Login to the os2 server.
2.  `cd /scratch/spring2018/32/linux-yocto-3.19/` - change into group folder's instance of the kernel repo.
3.  `git checkout os2` - checkout our base v3.19.2 tag branch.
4.  `git checkout -b hw#` - create a new branch from the os2 base brach named hw# (where # is the homework assignment number).
5.  `cp /scratch/files/config-3.19.2-yocto-standard .config` - copy the provided config file into the source tree.
6.  `make menuconfig` - opens menuconfig window for the project. Go into general settings and then change 'Local Version' to '-group32-hw#' (where # is the same homework assignment number from above), save to '.config' then exit.

### Build Kernal
1.  Login to the os2 server. Two terminals are perfered.
2.  `cd /scratch/spring2018/32/linux-yocto-3.19/` - change into group folder's instance of the kernel repo.
3.  `make clean` - cleans out any data from pervious builds (not nessacary from first time building).
4.  `make -j4 all > ../build.out` - this will begin the building of the kernal in the current terminal pipeing the output in the build.out file.
5.  From the other terminal, with its working directory at the base of the kernal source tree run: `tail -f ../build.out` - this views the kernal progress.

### Run Qemu VM
1.  Login to the os2 server. Two terminals are nessacary.
2.  `cd /scratch/spring2018/32` - change into group folder.
3.  `source environment-setup-i586-poky-linux` - setup the terminal environment for the vm.
6.  `qemu-system-i386 -gdb tcp::5532 -S -nographic -kernel bzImage-qemux65.bin -drive file=core-image-lsb-sdk-qemux86.ext4,if=virtio -enable-kvm -net none -usb -localtime --no-reboot --append "root=/dev/vda rw console=ttyS0 debug"` - launches the vm in a halted state (freezing the current terminal).
7.  `gdb -tui` - open gdb in tui mode.
8.  `target remote:5532` - select the remote vm target on the port set in the vm command.
9.  `continue` - continues the execution of the vm (on the other terminal).
10. Once displayed with the login prompt, on the other terminal: `root` - this is the password, press enter and then you are logged into the vm as the password is empty.
