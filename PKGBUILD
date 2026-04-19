# Maintainer: musqz <https://github.com/musqz>
pkgname=fittsmon
pkgver=1.1.0
pkgrel=1
pkgdesc="Lightweight X11 utility to bind mouse actions to screen corners and edges (multi-monitor)"
arch=('x86_64')
url="https://github.com/musqz/fittsmon"
license=('GPL-2.0-or-later')
depends=(
    'libxcb'
    'libxrandr'
    'glib2'
)
makedepends=(
    'xcb-proto'
)
source=("$pkgname-$pkgver.tar.gz::$url/archive/refs/tags/v$pkgver.tar.gz")
sha256sums=('SKIP')

build() {
    cd "$pkgname-$pkgver"
    make
}

package() {
    cd "$pkgname-$pkgver"
    make DESTDIR="$pkgdir" PREFIX=/usr install
}
