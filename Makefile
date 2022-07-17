clean:
	rm md2html
	rm generateblog
run:
	gcc ./md4c/md2html.c -o md2html -w
	gcc main.c -o generateblog