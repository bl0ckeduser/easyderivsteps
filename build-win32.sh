REL=1a

echo "MAIN BUILD...."
./build.sh

echo "ARCHIVING..."

rm -rf easyderivsteps*.zip easyderivsteps*-w/
mkdir easyderivsteps${REL}-w
cp -r compiled-xul-app/* easyderivsteps${REL}-w/
cp -r /media/0CCC9477CC945D30/symdiff/xulrunner-win32 easyderivsteps${REL}-w/code
cp winreadme easyderivsteps${REL}-w/README.TXT
unix2dos easyderivsteps${REL}-w/README.TXT
rm easyderivsteps${REL}-w/linux-run.sh
rm easyderivsteps${REL}-w/*.bat

cp /media/0CCC9477CC945D30/symdiff/winlauncher/*.exe easyderivsteps${REL}-w/

wine win7z/7zG.exe a easyderivsteps${REL}-w.exe -sfx easyderivsteps${REL}-w/

zip easyderivsteps${REL}-w.zip easyderivsteps${REL}-w.exe

#zip -r easyderivsteps11-w-WIPTEST.zip easyderivsteps${REL}-w/*
#zip -r easyderivsteps10-w.zip easyderivsteps10-w/*
