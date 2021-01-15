DESCRIPTION = "Heart Rate Monitor application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://heartrate.c"

S = "${WORKDIR}"

do_compile() {
	set CFLAGS -g
	${CC} ${CFLAGS}-lm  heartrate.c ${LDFLAGS} -o heartrate -lpthread
	unset CFLAGS
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 heartrate ${D}${bindir}
}
