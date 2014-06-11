rm -rf easyderivsteps10-lx.tar.gz easyderivsteps10-lx/
mkdir easyderivsteps10-lx
cp -r compiled-xul-app/* easyderivsteps10-lx/
#cp -r /media/0CCC9477CC945D30/symdiff/xulrunner-win32 easyderivsteps10-w/code
cp /media/0CCC9477CC945D30/symdiff/linuxbuild/xulrunner-21.0.en-US.linux-i686.tar.bz2 easyderivsteps10-lx/code/
cp linreadme easyderivsteps10-lx/README
rm easyderivsteps10-lx/*.sh
rm easyderivsteps10-lx/*.bat

#cp linlaunch easyderivsteps10-lx/easyderivsteps
cc -m32 linlauncher.c -o easyderivsteps10-lx/easyderivsteps

cd easyderivsteps10-lx/code/
tar -xjvf xulrunner*
rm *.tar.bz2
cd ../..

#tar -rvf easyderivsteps10-lx.tar easyderivsteps10-lx/
#7z a easyderivsteps10-lx.tar.7z easyderivsteps10-lx.

