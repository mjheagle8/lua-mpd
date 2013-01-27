# Contributor: mjheagle <mjheagle8@gmail.com>

pkgname=lua-mpd
pkgver=$(git hist | wc -l)
pkgrel=1
pkgdesc="multimedia control and status for mpd and mplayer"
url=""
arch=('i686' 'x86_64')
license=('MIT')
options=()
depends=('libmpdclient' 'lua' 'lua51')
source=()
md5sums=()

build() {
    cd $startdir

    make || return 1
    make PREFIX=/usr DESTDIR=$pkgdir install || return 1
}
