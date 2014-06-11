cat about.html | sed "s/%%%COMPILEDATE%%%/`date`/g" >tmp.htm

emcc *.c -O2 -s ASM_JS=0 --embed-file rules --embed-file pres-rules --shell-file template.html -s ALLOW_MEMORY_GROWTH=1 -o index.html -s EXPORTED_FUNCTIONS="['_jsdoit']"

cp index.html compiled-xul-app/code/chrome/content/
mv tmp.htm compiled-xul-app/code/chrome/content/about.html
cp help.html help-box.html about-box.html compiled-xul-app/code/chrome/content/
cp help_images/*.png compiled-xul-app/code/chrome/content/
