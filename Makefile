all: libmnemophonix.so mnemophonix

SOURCES=wav.c fingerprinting.c fft.c spectrogram.c frames.c haar.c

mnemophonix: main.c libmnemophonix.so
	$(CC) -lmnemophonix -L. main.c -Wl,-rpath,. -o mnemophonix -Wall -Wextra -pedantic -std=c99

libmnemophonix.so: $(SOURCES)
	$(CC) -fPIC $(SOURCES) -shared -o libmnemophonix.so -Wall -Wextra -pedantic -std=c99

clean:
	rm -f mnemophonix libmnemophonix.so
