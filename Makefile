
all:
	make -C makeheaders
	make -C lemon

clean:
	make -C makeheaders clean
	make -C lemon clean

