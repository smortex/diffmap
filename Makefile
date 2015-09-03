PROG=	diffmap

NO_MAN=

UNAME_S!=	uname -s
.if ${UNAME_S} == Linux
CFLAGS+=	-D_GNU_SOURCE
.endif

.include <bsd.prog.mk>
