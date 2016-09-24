/* check for theft-of-code... :| */

void derp(){
	emscripten_run_script("if (escape(document.getElementById('derp').innerHTML + document.getElementById('derp').style.display) != '%3Cfont%20size%3D%22-1%22%3E%28C%29%202013-2016%2C%20bl0ckeduser%3C/font%3E') { document.write(''); } ;");
}
