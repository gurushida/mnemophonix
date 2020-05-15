all: libmnemophonix.so mnemophonix genperm

SOURCES=wav.c fingerprinting.c fft.c logbins.c spectralimages.c haar.c rawfingerprints.c minhash.c permutations.c fingerprintio.c \
        resample.c audionormalizer.c hannwindow.c search.c ffmpeg.c lsh.c

mnemophonix: main.c libmnemophonix.so
	$(CC) -L. main.c -lmnemophonix -lm -Wl,-rpath,. -o mnemophonix -Wall -Wextra -pedantic

libmnemophonix.so: $(SOURCES)
	$(CC) -fPIC $(SOURCES) -lpthread -shared -o libmnemophonix.so -Wall -Wextra -pedantic

genperm: generatepermutations.c
	$(CC) generatepermutations.c -o genperm -Wall -Wextra -pedantic

clean:
	rm -f mnemophonix libmnemophonix.so
