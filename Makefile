prefix=/usr

CFLAGS=`pkg-config --cflags libfm libavcodec libavformat libavutil`
LDFLAGS=`pkg-config --libs libavcodec libavformat libavutil` -rpath /usr/lib -no-undefined -module -avoid-version
PLUGIN_NAME=pcmanfm-statusbar-ffmpeg-info

target=$(shell gcc -dumpmachine)
libdir=$(prefix)/lib/$(target)

all:
	libtool --mode=compile gcc -g $(CFLAGS) -c $(PLUGIN_NAME).c
	libtool --mode=link gcc $(LDFLAGS) $(PLUGIN_NAME).lo -o $(PLUGIN_NAME).la

install:
	mkdir -p $(DESTDIR)$(libdir)/pcmanfm
	libtool --mode=install install -c $(PLUGIN_NAME).la $(DESTDIR)$(libdir)/pcmanfm
	libtool --finish $(DESTDIR)$(libdir)/pcmanfm
